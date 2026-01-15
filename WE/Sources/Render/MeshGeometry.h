#pragma once
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXCollision.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include <fbxsdk.h>
#include "GeometryGenerator.h"
#include "DirectX/DXMath.h"
#include "DirectX/DXUtility.h"
#include "DirectX/DXException.h"
#include "Utility/String.h"
#include "Utility/Class.h"

struct FVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
	DirectX::XMFLOAT3 TangentU;
};

struct FSpriteVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Size;
};

struct FSubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox Bounds;
};

class FMeshGeometry
{
public:
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::vector<FSubmeshGeometry> DrawArgs;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

class FMeshGeometryManager
{
	SINGLETON(FMeshGeometryManager);
public:
	template <typename TVertex>
	inline void BuildMeshGeometryU16(
		std::string Name,
		const std::vector<TVertex>& Vertices,
		const std::vector<std::uint16_t>& Indices,
		const std::vector<FSubmeshGeometry>& Submesh,
		D3D_PRIMITIVE_TOPOLOGY PrimitiveType,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	template <typename TVertex>
	inline void BuildMeshGeometryU32(
		std::string Name,
		const std::vector<TVertex>& Vertices,
		const std::vector<std::uint32_t>& Indices,
		const std::vector<FSubmeshGeometry>& Submesh,
		D3D_PRIMITIVE_TOPOLOGY PrimitiveType,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);

private:
	void BuildMeshGeometries(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildMeshGeometryFromMeshData(
		std::string Name,
		const FGeometryGenerator::MeshData& MeshData,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildSkullMeshGeometry(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildBillboardPoints(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildRectangle(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	std::unordered_map<std::string, std::unique_ptr<FMeshGeometry>> mMeshGeometries;

	void LoadFbxs(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void LoadFbx(
		const std::string& Name,
		const std::string& FilePath,
		FbxManager* lSdkManager,
		FbxImporter* lImporter,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
public:
	inline FMeshGeometry* GetMeshGeometry(std::string Name)
	{
		return mMeshGeometries[Name].get();
	}
};

inline FMeshGeometryManager* GetMeshGeometryManager()
{
	return FMeshGeometryManager::GetInstance();
}

template<typename TVertex>
inline void FMeshGeometryManager::BuildMeshGeometryU16(
	std::string Name,
	const std::vector<TVertex>& Vertices,
	const std::vector<std::uint16_t>& Indices,
	const std::vector<FSubmeshGeometry>& Submesh,
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	const UINT VBByteSize = (UINT)Vertices.size() * sizeof(TVertex);
	const UINT IBByteSize = (UINT)Indices.size() * sizeof(std::uint16_t);

	std::unique_ptr<FMeshGeometry> Geometry = std::make_unique<FMeshGeometry>();
	Geometry->Name = Name;
	Geometry->PrimitiveType = PrimitiveType;
	THROW_IF_FAILED(D3DCreateBlob(VBByteSize, &Geometry->VertexBufferCPU));
	CopyMemory(Geometry->VertexBufferCPU->GetBufferPointer(), Vertices.data(), VBByteSize);

	THROW_IF_FAILED(D3DCreateBlob(IBByteSize, &Geometry->IndexBufferCPU));
	CopyMemory(Geometry->IndexBufferCPU->GetBufferPointer(), Indices.data(), IBByteSize);

	Geometry->VertexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Vertices.data(), VBByteSize, Geometry->VertexBufferUploader);

	Geometry->IndexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Indices.data(), IBByteSize, Geometry->IndexBufferUploader);

	Geometry->VertexByteStride = sizeof(TVertex);
	Geometry->VertexBufferByteSize = VBByteSize;
	Geometry->IndexFormat = DXGI_FORMAT_R16_UINT;
	Geometry->IndexBufferByteSize = IBByteSize;

	for (int i = 0; i < Submesh.size(); ++i)
	{
		Geometry->DrawArgs.push_back(Submesh[i]);
	}
	mMeshGeometries[Name] = std::move(Geometry);
}

template<typename TVertex>
inline void FMeshGeometryManager::BuildMeshGeometryU32(
	std::string Name,
	const std::vector<TVertex>& Vertices,
	const std::vector<std::uint32_t>& Indices,
	const std::vector<FSubmeshGeometry>& Submesh,
	D3D_PRIMITIVE_TOPOLOGY PrimitiveType,
	ID3D12Device* Device,
	ID3D12GraphicsCommandList* CommandList
)
{
	const UINT VBByteSize = (UINT)Vertices.size() * sizeof(TVertex);
	const UINT IBByteSize = (UINT)Indices.size() * sizeof(std::uint32_t);

	std::unique_ptr<FMeshGeometry> Geometry = std::make_unique<FMeshGeometry>();
	Geometry->Name = Name;
	Geometry->PrimitiveType = PrimitiveType;
	THROW_IF_FAILED(D3DCreateBlob(VBByteSize, &Geometry->VertexBufferCPU));
	CopyMemory(Geometry->VertexBufferCPU->GetBufferPointer(), Vertices.data(), VBByteSize);

	THROW_IF_FAILED(D3DCreateBlob(IBByteSize, &Geometry->IndexBufferCPU));
	CopyMemory(Geometry->IndexBufferCPU->GetBufferPointer(), Indices.data(), IBByteSize);

	Geometry->VertexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Vertices.data(), VBByteSize, Geometry->VertexBufferUploader);

	Geometry->IndexBufferGPU = FDXUtility::CreateDefaultBuffer(Device,
		CommandList, Indices.data(), IBByteSize, Geometry->IndexBufferUploader);

	Geometry->VertexByteStride = sizeof(TVertex);
	Geometry->VertexBufferByteSize = VBByteSize;
	Geometry->IndexFormat = DXGI_FORMAT_R32_UINT;
	Geometry->IndexBufferByteSize = IBByteSize;

	for (int i = 0; i < Submesh.size(); ++i)
	{
		Geometry->DrawArgs.push_back(Submesh[i]);
	}
	mMeshGeometries[Name] = std::move(Geometry);
}
