#include "SkeletalMesh.h"

#include "MeshGeometry.h"
#include "DirectX/DXResourceManager.h"
#include <filesystem>

FSkeletalMeshManager::FSkeletalMeshManager()
{
	// Load a skinned model.

	std::wstring CurrentPath = std::filesystem::current_path().c_str();
	std::wstring FilePathW = CurrentPath + L"\\..\\WE\\Resources\\Models\\soldier.m3d";
	std::string FilePath = WStringToString(FilePathW);

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