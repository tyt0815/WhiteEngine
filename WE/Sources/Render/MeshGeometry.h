#pragma once
#include <d3d12.h>
#include <DirectXCollision.h>
#include <string>
#include <vector>
#include <memory>
#include <wrl.h>
#include "GeometryGenerator.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"

enum EMeshGeometryType
{
	EMGT_Box,
	EMGT_Grid,
	EMGT_Sphere,
	EMGT_Cylinder,
	EMGT_Skull,
	EMGT_BillboardPoint,
	EMGT_None
};

struct FVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 TexC;
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
	EMeshGeometryType Type;

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

private:
	void BuildMeshGeometries(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildMeshGeometry(
		EMeshGeometryType Type,
		const std::vector<FVertex>& Vertices,
		const std::vector<std::uint32_t>& Indices,
		const std::vector<FSubmeshGeometry>& Submesh,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildMeshGeometryFromMeshData(
		EMeshGeometryType Type,
		const UGeometryGenerator::MeshData& MeshData,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildSkullMeshGeometry(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void BuildBillboardPoints(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	std::vector<std::unique_ptr<FMeshGeometry>> mMeshGeometries;
public:
	inline FMeshGeometry* GetMeshGeometry(std::uint64_t i)
	{
		return mMeshGeometries[i].get();
	}
};

inline FMeshGeometryManager* GetMeshGeometryManager()
{
	return FMeshGeometryManager::GetInstance();
}