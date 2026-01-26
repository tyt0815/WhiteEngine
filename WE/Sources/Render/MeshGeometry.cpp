#include "MeshGeometry.h"

#include <array>
#include <vector>
#include "GeometryGenerator.h"
#include "DirectX/DXResourceManager.h"
#include "Utility/FileIO.h"

#include "SkeletalMesh.h"

#pragma comment(lib, "libfbxsdk.lib")

FMeshGeometryManager::FMeshGeometryManager()
{

	GetDXResourceManagerPtr()->ExecuteAndFlushCommand(&FMeshGeometryManager::BuildMeshGeometries, this);
}

FMeshGeometryManager::~FMeshGeometryManager()
{

}

void FMeshGeometryManager::BuildMeshGeometries(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	FGeometryGenerator GeoGen;
	BuildMeshGeometryFromMeshData("Box", GeoGen.CreateBox(1.0f, 1.0f, 1.0f, 0), Device, CommandList);
	BuildMeshGeometryFromMeshData("Grid", GeoGen.CreateGrid(500.0f, 500.0f, 60, 40), Device, CommandList);
	BuildMeshGeometryFromMeshData("Sphere", GeoGen.CreateSphere(0.5f, 20, 20), Device, CommandList);
	BuildMeshGeometryFromMeshData("Cylinder", GeoGen.CreateCylinder(.5f, 0.5f, 1.0f, 20, 20), Device, CommandList);
	BuildMeshGeometryFromMeshData("Floor", GeoGen.CreateGrid(500.0f, 500.0f, 60, 40, 10), Device, CommandList);

	BuildBillboardPoints(Device, CommandList);
	BuildRectangle(Device, CommandList);

	LoadFbxs(Device, CommandList);;

	FSkeletalMesh* SkeletalMesh = FSkeletalMeshManager::GetInstance()->mSkeletalMesh.get();

	std::vector<SkinnedVertex> Vertices(SkeletalMesh->Vertices.size());
	for (size_t i = 0; i < SkeletalMesh->Vertices.size(); ++i)
	{
		Vertices[i].Pos = SkeletalMesh->Vertices[i].Pos;
		Vertices[i].Normal = SkeletalMesh->Vertices[i].Normal;
		Vertices[i].TexC = SkeletalMesh->Vertices[i].TexC;
		Vertices[i].TangentU = SkeletalMesh->Vertices[i].TangentU;
		Vertices[i].BoneWeights = SkeletalMesh->Vertices[i].BoneWeights;
		for (int j = 0; j < 4; ++j)
		{
			Vertices[i].BoneIndices[j] = SkeletalMesh->Vertices[i].BoneIndices[j];
		}
	}

	std::vector<FSubmeshGeometry> Submesh(SkeletalMesh->SkinnedSubsets.size());
	for (UINT i = 0; i < (UINT)SkeletalMesh->SkinnedSubsets.size(); ++i)
	{
		Submesh[i].IndexCount = (UINT)SkeletalMesh->SkinnedSubsets[i].FaceCount * 3;
		Submesh[i].StartIndexLocation = SkeletalMesh->SkinnedSubsets[i].FaceStart * 3;
		Submesh[i].BaseVertexLocation = 0;
	}

	BuildMeshGeometryU16(
		"Soldier",
		Vertices,
		SkeletalMesh->Indices,
		Submesh,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		Device,
		CommandList
	);
}

void FMeshGeometryManager::BuildMeshGeometryFromMeshData(
	std::string Name,
	const FGeometryGenerator::MeshData& MeshData,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	FSubmeshGeometry Submesh;
	Submesh.IndexCount = (UINT)MeshData.Indices32.size();
	Submesh.StartIndexLocation = 0;
	Submesh.BaseVertexLocation = 0;
	std::vector<FVertex> Vertices(MeshData.Vertices.size());
	for (size_t i = 0; i < MeshData.Vertices.size(); ++i)
	{

		Vertices[i].Pos = MeshData.Vertices[i].Position;
		Vertices[i].Normal = MeshData.Vertices[i].Normal;
		Vertices[i].TexC = MeshData.Vertices[i].TexC;
		Vertices[i].TangentU = MeshData.Vertices[i].TangentU;
	}
	std::vector<std::uint32_t> Indices;
	Indices.insert(Indices.end(), std::begin(MeshData.Indices32), std::end(MeshData.Indices32));
	BuildMeshGeometryU32(Name, Vertices, Indices, std::vector<FSubmeshGeometry>(1, Submesh), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Device, CommandList);
}

void FMeshGeometryManager::BuildSkullMeshGeometry(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	std::ifstream fin;
	ReadFile("Models/Skull.txt", fin);

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<FVertex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z >>
			vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<std::uint32_t> indices(3 * tcount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	//
	// Pack the indices of all the meshes into one index buffer.
	//
	FSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	BuildMeshGeometryU32("Skull", vertices, indices, std::vector<FSubmeshGeometry>(1, submesh), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Device, CommandList);
}

void FMeshGeometryManager::BuildBillboardPoints(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	constexpr int PointCount = 16;
	std::vector<FSpriteVertex> vertices(PointCount);
	for (UINT i = 0; i < PointCount; ++i)
	{
		float Offset = .5f;
		float x = FDXMath::RandF(-Offset, Offset);
		float z = FDXMath::RandF(-Offset, Offset);
		float y = 0.0f;

		// Move tree slightly above land height.
		y += 0.4f;

		vertices[i].Pos = XMFLOAT3(x, y, z);
		vertices[i].Size = XMFLOAT2(1.0f, 1.0f);
	}

	std::vector<std::uint16_t> indices(PointCount);
	for (int i = 0; i < PointCount; ++i)
	{
		indices[i] = i;
	}

	FSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	std::vector<FSubmeshGeometry> Submeshs = { submesh };

	BuildMeshGeometryU16(
		"BillboardPoint",
		vertices,
		indices,
		Submeshs,
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
		Device,
		CommandList
	);
}

void FMeshGeometryManager::BuildRectangle(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	struct FRectVertex
	{
		XMFLOAT2 Position;
		XMFLOAT2 TexC;
	};
	std::vector<FRectVertex> Vertices = {
		{XMFLOAT2(-1.f, -1.f), XMFLOAT2(0.f, 1.f)},
		{XMFLOAT2(-1.f, 1.f), XMFLOAT2(0.f, 0.f)},
		{XMFLOAT2(1.f, 1.f), XMFLOAT2(1.f, 0.f)},
		{XMFLOAT2(1.f, -1.f), XMFLOAT2(1.f, 1.f)}
	};

	std::vector<std::uint16_t> Indices = {
		0, 1, 2,
		0, 2, 3
	};


	FSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)Indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;
	std::vector<FSubmeshGeometry> Submeshs = { submesh };

	BuildMeshGeometryU16(
		"Rectangle",
		Vertices,
		Indices,
		Submeshs,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		Device,
		CommandList
	);
}

void FMeshGeometryManager::LoadFbxs(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	// 1. FBX SDK 관리자 및 임포터 초기화
	FbxManager* lSdkManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	std::string FBXDir = SOLUTION_DIR;
	FBXDir += "/Resources/FBX";
	LoadFbx("Ring", FBXDir + "/Ring.fbx", lSdkManager, lImporter, Device, CommandList);;


	lSdkManager->Destroy();
}

void FMeshGeometryManager::LoadFbx(
	const std::string& Name,
	const std::string& FilePath,
	fbxsdk::FbxManager* lSdkManager,
	fbxsdk::FbxImporter* lImporter,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	std::string fileName(FilePath.begin(), FilePath.end());

	if (!lImporter->Initialize(fileName.c_str(), -1, lSdkManager->GetIOSettings())) {
		return;
	}

	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
	lImporter->Import(lScene);

	// Importer는 Import 직후 Destroy해도 씬 데이터는 lScene에 남습니다.
	lImporter->Destroy();

	FbxNode* lRootNode = lScene->GetRootNode();
	if (!lRootNode) return;

	// 전체 데이터를 담을 컨테이너
	std::vector<FVertex> AllVertices;
	std::vector<std::uint32_t> AllIndices;
	std::vector<FSubmeshGeometry> Submeshes;

	// 씬 전체를 순회하며 메시 노드 추출 (재귀 대신 간단한 스택/루프 구조)
	std::vector<FbxNode*> NodeStack;
	NodeStack.push_back(lRootNode);

	while (!NodeStack.empty())
	{
		FbxNode* currentNode = NodeStack.back();
		NodeStack.pop_back();

		// 자식 노드들을 스택에 추가
		for (int i = 0; i < currentNode->GetChildCount(); ++i)
			NodeStack.push_back(currentNode->GetChild(i));

		FbxMesh* lMesh = currentNode->GetMesh();
		if (lMesh)
		{
			FbxGeometryElementUV* leUV = lMesh->GetElementUV(0);
			FbxGeometryElementNormal* leNormal = lMesh->GetElementNormal(0);
			FbxGeometryElementTangent* leTangent = lMesh->GetElementTangent(0);

			FSubmeshGeometry Submesh;
			// 현재까지 쌓인 데이터 이후부터가 이 서브메시의 시작점
			Submesh.BaseVertexLocation = (UINT)AllVertices.size();
			Submesh.StartIndexLocation = (UINT)AllIndices.size();

			// 1. 정점 데이터 추출
			int lControlPointsCount = lMesh->GetControlPointsCount();
			FbxVector4* lControlPoints = lMesh->GetControlPoints();

			for (int i = 0; i < lControlPointsCount; ++i)
			{
				FVertex v;
				v.Pos = { (float)lControlPoints[i][0], (float)lControlPoints[i][1], (float)lControlPoints[i][2] };

				// 기본값 채우기 (노멀 추출 로직은 나중에 추가 가능)
				v.Normal = { 0.0f, 1.0f, 0.0f };
				v.TangentU = { 1.0f, 0.0f, 0.0f };
				v.TexC = { 0.0f, 0.0f }; // 기본값


				// --- 1. Normal 추출 ---
				if (leNormal)
				{
					int nIdx = (leNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? i : i; // 단순화된 인덱스
					if (leNormal->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						nIdx = leNormal->GetIndexArray().GetAt(i);

					FbxVector4 norm = leNormal->GetDirectArray().GetAt(nIdx);
					v.Normal = { (float)norm[0], (float)norm[1], (float)norm[2] };
				}

				// --- 2. UV 추출 ---
				if (leUV)
				{
					int uvIdx = (leUV->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? i : i;
					if (leUV->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						uvIdx = leUV->GetIndexArray().GetAt(i);

					FbxVector2 uv = leUV->GetDirectArray().GetAt(uvIdx);
					v.TexC = { (float)uv[0], 1.0f - (float)uv[1] }; // DX 좌표계 반전
				}

				// --- 3. Tangent 추출 ---
				if (leTangent)
				{
					int tIdx = (leTangent->GetMappingMode() == FbxGeometryElement::eByControlPoint) ? i : i;
					if (leTangent->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						tIdx = leTangent->GetIndexArray().GetAt(i);

					FbxVector4 tan = leTangent->GetDirectArray().GetAt(tIdx);
					v.TangentU = { (float)tan[0], (float)tan[1], (float)tan[2] };
				}
				else {
					v.TangentU = { 1.0f, 0.0f, 0.0f }; // 없으면 기본값
				}

				AllVertices.push_back(v);
			}

			// 2. 인덱스 데이터 추출
			int lPolygonCount = lMesh->GetPolygonCount();
			for (int i = 0; i < lPolygonCount; i++)
			{
				// FBX Export시 Triangulate를 체크했다면 항상 3개씩 들어옵니다.
				for (int j = 0; j < 3; j++)
				{
					AllIndices.push_back((std::uint32_t)lMesh->GetPolygonVertex(i, j));
				}
			}

			// 이번 메시에 추가된 인덱스 개수 계산
			Submesh.IndexCount = (UINT)AllIndices.size() - Submesh.StartIndexLocation;

			// 유효한 메시 데이터가 있다면 서브메시 목록에 추가
			if (Submesh.IndexCount > 0)
				Submeshes.push_back(Submesh);
		}
	}

	// 3. 통합된 데이터로 메시 지오메트리 빌드
	if (!AllVertices.empty())
	{
		BuildMeshGeometryU32(
			Name,
			AllVertices,
			AllIndices,
			Submeshes,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			Device,
			CommandList
		);
	}

	// Scene 데이터 정리
	lScene->Destroy();
}