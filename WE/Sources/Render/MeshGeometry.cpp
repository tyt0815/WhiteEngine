#include "MeshGeometry.h"

#include <array>
#include <D3Dcompiler.h>
#include <vector>
#include "GeometryGenerator.h"
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"
#include "Utility/FileIO.h"

FMeshGeometryManager::FMeshGeometryManager()
{
	mMeshGeometries.resize(EMGT_None);
	GetDXResourceManagerPtr()->FlushAndExecuteCommand(&FMeshGeometryManager::BuildMeshGeometries, this);
}

FMeshGeometryManager::~FMeshGeometryManager()
{

}

void FMeshGeometryManager::BuildMeshGeometries(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	UGeometryGenerator GeoGen;
	BuildMeshGeometryFromMeshData(EMGT_Box, GeoGen.CreateBox(1.0f, 1.0f, 1.0f, 0), Device, CommandList);
	BuildMeshGeometryFromMeshData(EMGT_Grid, GeoGen.CreateGrid(500.0f, 500.0f, 60, 40), Device, CommandList);
	BuildMeshGeometryFromMeshData(EMGT_Sphere, GeoGen.CreateSphere(0.5f, 20, 20), Device, CommandList);
	BuildMeshGeometryFromMeshData(EMGT_Cylinder, GeoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20), Device, CommandList);
	BuildSkullMeshGeometry(Device, CommandList);
	BuildBillboardPoints(Device, CommandList);
}

void FMeshGeometryManager::BuildMeshGeometry(
	EMeshGeometryType Type,
	const std::vector<FVertex>& Vertices,
	const std::vector<std::uint32_t>& Indices,
	const std::vector<FSubmeshGeometry>& Submesh,
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	const UINT VBByteSize = (UINT)Vertices.size() * sizeof(FVertex);
	const UINT IBByteSize = (UINT)Indices.size() * sizeof(std::uint32_t);

	std::unique_ptr<FMeshGeometry> Geometry = std::make_unique<FMeshGeometry>();
	Geometry->Type = Type;
	Geometry->PrimitiveType = PrimitiveType;
	THROW_IF_FAILED(D3DCreateBlob(VBByteSize, &Geometry->VertexBufferCPU));
	CopyMemory(Geometry->VertexBufferCPU->GetBufferPointer(), Vertices.data(), VBByteSize);

	THROW_IF_FAILED(D3DCreateBlob(IBByteSize, &Geometry->IndexBufferCPU));
	CopyMemory(Geometry->IndexBufferCPU->GetBufferPointer(), Indices.data(), IBByteSize);

	Geometry->VertexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Vertices.data(), VBByteSize, Geometry->VertexBufferUploader);

	Geometry->IndexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Indices.data(), IBByteSize, Geometry->IndexBufferUploader);

	Geometry->VertexByteStride = sizeof(FVertex);
	Geometry->VertexBufferByteSize = VBByteSize;
	Geometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	Geometry->IndexBufferByteSize = IBByteSize;

	for (int i = 0; i < Submesh.size(); ++i)
	{
		Geometry->DrawArgs.push_back(Submesh[i]);
	}
	mMeshGeometries[Type] = std::move(Geometry);
}

void FMeshGeometryManager::BuildMeshGeometryFromMeshData(
	EMeshGeometryType Type,
	const UGeometryGenerator::MeshData& MeshData,
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
	}
	std::vector<std::uint32_t> Indices;
	Indices.insert(Indices.end(), std::begin(MeshData.Indices32), std::end(MeshData.Indices32));
	BuildMeshGeometry(Type, Vertices, Indices, std::vector<FSubmeshGeometry>(1, Submesh), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Device, CommandList);
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
	BuildMeshGeometry(EMGT_Skull, vertices, indices, std::vector<FSubmeshGeometry>(1, submesh), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Device, CommandList);
}

void FMeshGeometryManager::BuildBillboardPoints(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	struct FSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	constexpr int PointCount = 16;
	std::array<FSpriteVertex, PointCount> vertices;
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

	std::array<std::uint16_t, 16> indices;
	for (int i = 0; i < PointCount; ++i)
	{
		indices[i] = i;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(FSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<FMeshGeometry>();
	geo->Type = EMGT_BillboardPoint;

	THROW_IF_FAILED(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	THROW_IF_FAILED(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(FSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;
	geo->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

	FSubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;


	geo->DrawArgs.push_back(submesh);
	mMeshGeometries[EMGT_BillboardPoint] = std::move(geo);
}
