#include "MeshGeometry.h"

#include <array>
#include <vector>
#include "GeometryGenerator.h"
#include "DirectX/DXResourceManager.h"
#include "Utility/FileIO.h"

#include "SkeletalMesh.h"

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
	BuildMeshGeometryFromMeshData("Cylinder", GeoGen.CreateCylinder(.5f, 0.1f, .5f, 20, 20), Device, CommandList);
	BuildMeshGeometryFromMeshData("Floor", GeoGen.CreateBox(100.0f, 1.0f, 100, 0), Device, CommandList);

	BuildBillboardPoints(Device, CommandList);
	BuildRectangle(Device, CommandList);

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