#pragma once

#include <d3d12.h>
#include <unordered_map>
#include <vector>

#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "FrameResource.h"

class WWorld;

struct FRenderData
{
	FFrameResource* FrameResource;
};

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
	virtual bool Initialize();
	virtual void Render(const FRenderData& RenderData);

private:
	void BuildDescriptorHeaps();
	void BuildShaderResources();
	void BuildRootSignature();
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	void DrawActors(const std::vector<AActor*>& DrawTargets, FFrameResource* TargetFrameResource);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SRVHeap;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature;
	ID3D12Fence* Fence = nullptr;
	ID3D12Device* Device = nullptr;
	ID3D12CommandQueue* CommandQueue = nullptr;
	ID3D12CommandAllocator* CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* CommandList = nullptr;
	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayouts;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> Shaders;
	std::vector<std::unique_ptr<FFrameResource>> FrameResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> PipelineStateObjects;

	int TargetFrameResourceIndex = 0;

	bool bWireFrame = false;
};