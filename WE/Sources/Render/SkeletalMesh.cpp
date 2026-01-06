#include "SkeletalMesh.h"

#include "MeshGeometry.h"
#include "DirectX/DXResourceManager.h"

FSkeletalMeshManager::FSkeletalMeshManager()
{
	// Load a skinned model.

	
	std::string FilePath = "./Resources/Models/soldier.m3d";

	mSkeletalMesh = std::make_unique<FSkeletalMesh>();

	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(FilePath, mSkeletalMesh->Vertices, mSkeletalMesh->Indices,
		mSkeletalMesh->SkinnedSubsets, mSkeletalMesh->SkinnedMats, mSkeletalMesh->SkinnedInfo);

	mSkeletalMesh->SkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkeletalMesh->SkinnedModelInst->SkinnedInfo = &mSkeletalMesh->SkinnedInfo;
	mSkeletalMesh->SkinnedModelInst->FinalTransforms.resize(mSkeletalMesh->SkinnedInfo.BoneCount());
	mSkeletalMesh->SkinnedModelInst->ClipName = "Take1";
	mSkeletalMesh->SkinnedModelInst->TimePos = 0.0f;
}

FSkeletalMeshManager::~FSkeletalMeshManager()
{
}