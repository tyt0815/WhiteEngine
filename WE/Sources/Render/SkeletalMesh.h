#pragma once

#include "Utility/Class.h"
#include "SkinnedData.h"
#include "LoadM3d.h"

#include <memory>

struct SkinnedVertex
{
    DirectX::XMFLOAT3 Pos;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 TexC;
    DirectX::XMFLOAT3 TangentU;
    DirectX::XMFLOAT3 BoneWeights;
    UINT BoneIndices[4];
};

struct SkinnedModelInstance
{
    SkinnedData* SkinnedInfo = nullptr;
    std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
    std::string ClipName;
    float TimePos = 0.0f;

    // Called every frame and increments the time position, interpolates the 
    // animations for each bone based on the current animation clip, and 
    // generates the final transforms which are ultimately set to the effect
    // for processing in the vertex shader.
    void UpdateSkinnedAnimation(float dt)
    {
        TimePos += dt;

        // Loop animation
        if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
            TimePos = 0.0f;

        // Compute the final transforms for this time position.
        SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
    }
};

struct FSkeletalMesh
{
    std::unique_ptr<SkinnedModelInstance> SkinnedModelInst;
    std::vector<M3DLoader::Subset> SkinnedSubsets;
    std::vector<M3DLoader::M3dMaterial> SkinnedMats;
    std::vector<M3DLoader::SkinnedVertex> Vertices;
    std::vector<std::uint16_t> Indices;
    SkinnedData SkinnedInfo;
};

class FSkeletalMeshManager
{
	SINGLETON(FSkeletalMeshManager);
public:
	std::unique_ptr<FSkeletalMesh> mSkeletalMesh;
};