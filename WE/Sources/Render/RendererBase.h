#pragma once

#include "DirectX/DXWindow.h"
#include "FrameResource.h"

class WWorld;
class AActor;

extern const int NumFrameResources;

class FRendererBase : FNoncopyable
{
public:
	FRendererBase(WWorld* World);
	virtual bool Initialize();
	virtual void Render();

protected:
	virtual void BuildDescriptorHeaps() = 0;
	virtual void BuildConstantBuffers() = 0;
	virtual void BuildRootSignature() = 0;
	virtual void BuildShaderAndInputLayout() = 0;
	virtual void BuildPipelineStateObject() = 0;
	void BuildFrameResources();
	void SetTargetFrameResource();
	void UpdateCamera();

	vector<unique_ptr<FFrameResource>> FrameResources;
	vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	unordered_map<string, ComPtr<ID3D12PipelineState>> PipelineStateObjects;
	FFrameResource* TargetFrameResource = nullptr;
	FDXWindow* const DXWindow;
	int TargetFrameResourceIndex = 0;
	

	WWorld* World;
};