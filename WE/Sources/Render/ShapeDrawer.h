#pragma once

#include <d3d12.h>
#include <vector>

std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingSphereInputLayouts();
void DrawSphere(ID3D12GraphicsCommandList* CommandList);

std::vector<D3D12_INPUT_ELEMENT_DESC> GetDrawingRectInputLayouts();
void DrawRect(ID3D12GraphicsCommandList* CommandList);


// DrawMeshGeometry 함수의 입력 파라미터.
struct FMeshDrawInfo
{
	D3D12_GPU_VIRTUAL_ADDRESS ObjectConstantBufferAddress;
	D3D12_GPU_VIRTUAL_ADDRESS SubmeshConstantBufferAddress;
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	UINT IndexCount;
	UINT StartIndexLocation;
	UINT BaseVertexLocation;
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology;
};

void DrawMeshGeometry(ID3D12GraphicsCommandList* CommandList, const FMeshDrawInfo& MeshDrawInfo);