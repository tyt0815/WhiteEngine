#include "SkeletalMesh.h"

#include "MeshGeometry.h"
#include "LoadM3d.h"
#include "DirectX/DXResourceManager.h"

FSkeletalMeshManager::FSkeletalMeshManager()
{
	// Load a skinned model.

	std::vector<M3DLoader::SkinnedVertex> vertices;
	std::vector<std::uint16_t> indices;
	std::string FilePath = "./Resources/Models/soldier.m3d";
	std::vector<M3DLoader::Subset> mSkinnedSubsets;
	std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
	SkinnedData mSkinnedInfo;

	M3DLoader m3dLoader;
	m3dLoader.LoadM3d(FilePath, vertices, indices,
		mSkinnedSubsets, mSkinnedMats, mSkinnedInfo);

	mSkinnedModelInst = std::make_unique<SkinnedModelInstance>();
	mSkinnedModelInst->SkinnedInfo = &mSkinnedInfo;
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedInfo.BoneCount());
	mSkinnedModelInst->ClipName = "Take1";
	mSkinnedModelInst->TimePos = 0.0f;

	std::vector<FSubmeshGeometry> Submesh(mSkinnedSubsets.size());
	for (UINT i = 0; i < (UINT)mSkinnedSubsets.size(); ++i)
	{
		Submesh[i].IndexCount = (UINT)mSkinnedSubsets[i].FaceCount * 3;
		Submesh[i].StartIndexLocation = mSkinnedSubsets[i].FaceStart * 3;
		Submesh[i].BaseVertexLocation = 0;
	}

	FMeshGeometryManager::GetInstance()->BuildMeshGeometryU16(
		"Soldier",
		vertices,
		indices,
		Submesh,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		FDXResourceManager::GetInstance()->GetDevicePtr(),
		FDXResourceManager::GetInstance()->GetCommandListPtr()
	);

	//auto geo = std::make_unique<FMeshGeometry>();
	//geo->Name = mSkinnedModelFilename;

	//ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	//CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	//ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	//CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
	//	mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	//geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
	//	mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	//geo->VertexByteStride = sizeof(SkinnedVertex);
	//geo->VertexBufferByteSize = vbByteSize;
	//geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	//geo->IndexBufferByteSize = ibByteSize;

	//for (UINT i = 0; i < (UINT)mSkinnedSubsets.size(); ++i)
	//{
	//	SubmeshGeometry submesh;
	//	std::string name = "sm_" + std::to_string(i);

	//	submesh.IndexCount = (UINT)mSkinnedSubsets[i].FaceCount * 3;
	//	submesh.StartIndexLocation = mSkinnedSubsets[i].FaceStart * 3;
	//	submesh.BaseVertexLocation = 0;

	//	geo->DrawArgs[name] = submesh;
	//}

	//mGeometries[geo->Name] = std::move(geo);
	
}

FSkeletalMeshManager::~FSkeletalMeshManager()
{
}