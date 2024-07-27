#pragma once

#include "RendererBase.h"

class FForwardRenderer : public FRendererBase
{
	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};
	typedef FRendererBase Super;
public:
	FForwardRenderer(WWorld* World): Super(World) {}
	virtual void Render() override;

protected:
	virtual void BuildDescriptorHeaps() override;
	virtual void BuildConstantBuffers() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShaderAndInputLayout() override;
	virtual void BuildPipelineStateObject() override;

	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12DescriptorHeap> CBVHeap;

	bool bWireFrame;
};