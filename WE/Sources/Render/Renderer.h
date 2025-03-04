#pragma once

#include <d3d12.h>
#include <unordered_map>
#include <vector>

#include "Runtime/Object/Actor.h"
#include "Utility/Class.h"
#include "FrameResource.h"

class FDXDeviceManager;
class WWorld;
//class IDXGIFactory4;
//class IDXGISwapChain;
//class ID3D12Fence;
//class ID3D12DescriptorHeap;
//class ID3D12Resource;

constexpr int FRAME_RESOURCES_NUM = 3;

class FRenderer : FNoncopyable
{
	enum class EPipelineState
	{
		EPS_Opaque,
		EPS_WireFrame,
		EPS_Transparency,
		EPS_AlphaTest,
		EPS_Billboard,
		EPS_None
	};
public:
	FRenderer();
	virtual bool Initialize(WWorld* InWorld);
	virtual void Render(class UTimer* Timer);
	class WViewCamera* Camera = nullptr;

private:
	void BuildFrameResources();
	void BuildDescriptorHeaps();
	void BuildShaderResources();
	void BuildRootSignature();
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	void SetTargetFrameResource();
	void UpdatePassConstantBuffers(UTimer* Timer);
	void UpdateObjectConstantBuffer();
	void UpdateMaterialConstantBuffer();
	void DrawActors(const std::vector<AActor*>& DrawTargets);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SRVHeap;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
	ID3D12Fence* Fence = nullptr;
	ID3D12Device* Device = nullptr;
	ID3D12CommandQueue* CommandQueue = nullptr;
	ID3D12CommandAllocator* CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* CommandList = nullptr;
	WWorld* World;
	FFrameResource* TargetFrameResource = nullptr;
	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayouts;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> Shaders;
	std::vector<std::unique_ptr<FFrameResource>> FrameResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> PipelineStateObjects;

	int TargetFrameResourceIndex = 0;

	bool bWireFrame = false;
};