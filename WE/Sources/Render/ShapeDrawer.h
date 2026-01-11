#pragma once

#include <d3d12.h>
#include <vector>
#include <DirectXMath.h>
#include <wrl.h>
#include <unordered_map>
#include "Utility/Class.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class FShapeDrawer final
{
	SINGLETON(FShapeDrawer);
	
	struct FShapeDrawerConstantBuffer
	{
		XMFLOAT4X4 ViewProjMatrix;
	};

public:
	void DrawLine3D(ID3D12GraphicsCommandList* CommandList, XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT4 Color);

private:
	void BuildRootSignature();

	void ComplieShaders();

	void BuildInputLayouts();

	void BuildPipelineStates();

	void BuildVertexBuffers(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

	ComPtr<ID3D12RootSignature> mRootSignature;

	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPipelineStates;

	ComPtr<ID3DBlob> VertexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
};

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