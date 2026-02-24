#pragma once

#include "Asset.h"
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>

class FBinaryWriter;
class FBinaryReader;

struct FSkeletalVertex {
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 UV;
    float Weights[4] = { 0, 0, 0, 0 };
    int   Indices[4] = { -1, -1, -1, -1 };
};

struct FJoint {
    std::string Name;
    int ParentIndex = -1;
    XMFLOAT4X4 InverseBindPose;
};

class FSkeletalMeshAsset : public FAsset
{
    typedef FAsset Super;

public:
    FSkeletalMeshAsset() {}
    virtual ~FSkeletalMeshAsset() override {}

    virtual bool LoadAsset(const std::wstring& FilePath) override;

protected:
    bool OnCompile(const std::wstring& SrcPath, std::vector<unsigned char>& OutBuffer);
    bool SmartLoad(const std::wstring& SourcePath, std::vector<unsigned char>& RawBuffer);

private:
    // FBX 파싱 내부 로직
    //void ProcessNode(FbxNode* Node, int ParentIndex);
    void ProcessMesh(FbxMesh* Mesh);
    void ProcessSkeleton(FbxNode* Node);

    void Serialize(FBinaryWriter& Writer);
    void Deserialize(FBinaryReader& Reader);

private:
    std::vector<FJoint> mSkeleton;
    std::vector<FSkeletalVertex> mVertices;
    std::vector<unsigned int> mIndices;
    std::unordered_map<std::string, int> mJointMap;

public:
    __forceinline const std::vector<FJoint>& GetSkeleton() const 
    { 
        return mSkeleton; 
    }

    __forceinline const std::vector<FSkeletalVertex>& GetVertices() const
    { 
        return mVertices; 
    }

    __forceinline const std::vector<unsigned int>& GetIndices() const
    { 
        return mIndices; 
    }

    __forceinline const std::unordered_map<std::string, int>& GetJointMap() const
    { 
        return mJointMap; 
    }
};