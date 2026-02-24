#include "SkeletalMeshAsset.h"
#include "Utility/Serialization.h"
#include "Utility/FileIO.h"
#include "Utility/String.h"
#include <filesystem>

// FBX SDK의 기하구조 변환기 등을 사용하기 위해 추가
#include <fbxsdk/utils/fbxgeometryconverter.h>

using namespace fbxsdk;

bool FSkeletalMeshAsset::LoadAsset(const std::wstring& FilePath)
{
    std::vector<unsigned char> RawBuffer;
    if (!SmartLoad(FilePath, RawBuffer))
    {
        return false;
    }

    FBinaryReader Reader(RawBuffer);
    Deserialize(Reader);

    return true;
}

bool FSkeletalMeshAsset::SmartLoad(const std::wstring& SourcePath, std::vector<unsigned char>& RawBuffer)
{
    std::filesystem::path p(SourcePath);
    std::filesystem::path binFolder = p.parent_path() / "bin";
    std::filesystem::create_directories(binFolder);

    std::wstring BinaryPath = (binFolder / (p.filename().wstring() + L".bin")).wstring();

    if (CheckIfNeedCompile(SourcePath, BinaryPath))
    {
        std::vector<unsigned char> Buffer;
        if (!OnCompile(SourcePath, Buffer))
        {
            return false;
        }
        FileIO::SaveBufferToFile(BinaryPath, Buffer);
    }

    return FileIO::LoadBufferFromFile(BinaryPath, RawBuffer);
}

bool FSkeletalMeshAsset::OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer)
{
    // --- 1. FBX SDK 초기화 ---
    FbxManager* Manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(Manager, IOSROOT);
    Manager->SetIOSettings(ios);

    FbxImporter* Importer = FbxImporter::Create(Manager, "");
    
    std::string path = WStringToString(SrcPath);

    if (!Importer->Initialize(path.c_str(), -1, Manager->GetIOSettings()))
    {
        Manager->Destroy();
        return false;
    }

    FbxScene* Scene = FbxScene::Create(Manager, "ImportScene");
    Importer->Import(Scene);
    Importer->Destroy();

    // --- 2. 씬 최적화 및 변환 ---
    // 모든 폴리곤을 삼각형으로 변환 (인덱스 버퍼 처리를 위해 필수)
    FbxGeometryConverter Converter(Manager);
    Converter.Triangulate(Scene, true);

    // 좌표계를 DirectX(Y-Up, Left-Handed)에 맞게 변환
    fbxsdk::FbxAxisSystem(fbxsdk::FbxAxisSystem::eDirectX).ConvertScene(Scene);

    // --- 3. 데이터 추출 ---
    mSkeleton.clear();
    mJointMap.clear();
    mVertices.clear();
    mIndices.clear();

    // 3-1. 스켈레톤 계층 구조 파싱
    ProcessSkeleton(Scene->GetRootNode());

    // 3-2. 메시 및 스킨 가중치 파싱
    FbxNode* RootNode = Scene->GetRootNode();
    for (int i = 0; i < RootNode->GetChildCount(); i++)
    {
        FbxNode* ChildNode = RootNode->GetChild(i);
        if (FbxMesh* Mesh = ChildNode->GetMesh())
        {
            ProcessMesh(Mesh);
        }
    }

    // --- 4. 바이너리로 굽기 ---
    FBinaryWriter Writer(OutBuffer);
    Serialize(Writer);

    Scene->Destroy();
    Manager->Destroy();

    return true;
}

void FSkeletalMeshAsset::ProcessSkeleton(FbxNode* Node)
{
    FbxNodeAttribute* Attr = Node->GetNodeAttribute();
    if (Attr && Attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
    {
        FJoint Joint;
        Joint.Name = Node->GetName();

        // 부모 조인트 인덱스 찾기
        Joint.ParentIndex = -1;
        FbxNode* ParentNode = Node->GetParent();
        if (ParentNode)
        {
            auto it = mJointMap.find(ParentNode->GetName());
            if (it != mJointMap.end()) Joint.ParentIndex = it->second;
        }

        mJointMap[Joint.Name] = (int)mSkeleton.size();
        mSkeleton.push_back(Joint);
    }

    for (int i = 0; i < Node->GetChildCount(); i++)
    {
        ProcessSkeleton(Node->GetChild(i));
    }
}

void FSkeletalMeshAsset::ProcessMesh(FbxMesh* Mesh)
{
    // 제어점(Control Points) 기반으로 기본 정점 데이터 생성
    int CPCount = Mesh->GetControlPointsCount();
    std::vector<FSkeletalVertex> TempVertices(CPCount);
    FbxVector4* CPs = Mesh->GetControlPoints();

    for (int i = 0; i < CPCount; i++)
    {
        TempVertices[i].Pos = XMFLOAT3((float)CPs[i][0], (float)CPs[i][1], (float)CPs[i][2]);
    }

    // 스킨 정보 파싱 (Bone Indices & Weights)
    int DeformerCount = Mesh->GetDeformerCount();
    for (int i = 0; i < DeformerCount; i++)
    {
        FbxSkin* Skin = (FbxSkin*)Mesh->GetDeformer(i, FbxDeformer::eSkin);
        if (!Skin) continue;

        int ClusterCount = Skin->GetClusterCount();
        for (int j = 0; j < ClusterCount; j++)
        {
            FbxCluster* Cluster = Skin->GetCluster(j);
            FbxNode* BoneNode = Cluster->GetLink();
            if (!BoneNode) continue;

            int JointIndex = mJointMap[BoneNode->GetName()];

            // Inverse Bind Pose 행렬 추출
            FbxAMatrix LinkMatrix;
            Cluster->GetTransformLinkMatrix(LinkMatrix);
            FbxAMatrix IBP = LinkMatrix.Inverse();

            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    mSkeleton[JointIndex].InverseBindPose(r, c) = (float)IBP.Get(r, c);

            // 가중치가 적용된 정점들 처리
            int* Indices = Cluster->GetControlPointIndices();
            double* Weights = Cluster->GetControlPointWeights();
            for (int k = 0; k < Cluster->GetControlPointIndicesCount(); k++)
            {
                int CPIdx = Indices[k];
                for (int slot = 0; slot < 4; slot++)
                {
                    if (TempVertices[CPIdx].Indices[slot] == -1)
                    {
                        TempVertices[CPIdx].Indices[slot] = JointIndex;
                        TempVertices[CPIdx].Weights[slot] = (float)Weights[k];
                        break;
                    }
                }
            }
        }
    }

    // 인덱스 버퍼 처리 (삼각형화된 메시 기준)
    int PolyCount = Mesh->GetPolygonCount();
    for (int i = 0; i < PolyCount; i++)
    {
        for (int j = 0; j < 3; j++) // Triangulated
        {
            mIndices.push_back(Mesh->GetPolygonVertex(i, j));
        }
    }
    mVertices = std::move(TempVertices);
}

void FSkeletalMeshAsset::Serialize(FBinaryWriter& Writer)
{
    // 스켈레톤
    Writer << (int)mSkeleton.size();
    for (const auto& Joint : mSkeleton)
    {
        Writer << Joint.Name << Joint.ParentIndex;
        Writer.WriteRaw(&Joint.InverseBindPose, 1);
    }

    // 정점 데이터
    Writer << (int)mVertices.size();
    Writer.WriteRaw(mVertices.data(), mVertices.size());

    // 인덱스 데이터
    Writer << (int)mIndices.size();
    Writer.WriteRaw(mIndices.data(), mIndices.size());
}

void FSkeletalMeshAsset::Deserialize(FBinaryReader& Reader)
{
    int SkeletonSize;
    Reader >> SkeletonSize;
    mSkeleton.resize(SkeletonSize);
    for (int i = 0; i < SkeletonSize; ++i)
    {
        Reader >> mSkeleton[i].Name >> mSkeleton[i].ParentIndex;
        Reader.ReadRaw(&mSkeleton[i].InverseBindPose, 1);
        mJointMap[mSkeleton[i].Name] = i;
    }

    int VertexCount;
    Reader >> VertexCount;
    mVertices.resize(VertexCount);
    Reader.ReadRaw(mVertices.data(), VertexCount);

    int IndexCount;
    Reader >> IndexCount;
    mIndices.resize(IndexCount);
    Reader.ReadRaw(mIndices.data(), IndexCount);
}