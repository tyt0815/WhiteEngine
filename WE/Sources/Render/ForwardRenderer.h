#pragma once

#include "RendererBase.h"

class FForwardRenderer : public FRendererBase
{
	typedef FRendererBase Super;
public:
	FForwardRenderer(): Super() {}
	virtual bool Initialize() override;

protected:
	enum class EPipelineState
	{
		EPS_Opaque,
		EPS_WireFrame,
		EPS_Transparency,
		EPS_AlphaTest,
		EPS_Billboard,
		EPS_None
	};
	virtual void Render(UTimer* Timer) override;
	virtual void BuildDescriptorHeaps() override;
	virtual void BuildConstantBuffers() override {};
	virtual void BuildShaderResources() override;
	virtual void BuildRootSignature() override;
	virtual void BuildShaderAndInputLayout() override;
	virtual void BuildPipelineStateObject() override;

	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12DescriptorHeap> SRVHeap;
private:
	void UpdatePassConstantBuffers(UTimer* Timer);
	void UpdateObjectConstantBuffer();
	void UpdateMaterialConstantBuffer();
	void DrawActors(const vector<AActor*>& Actors);

	UINT MaterialConstantBufferViewOffset = 0;
	UINT PassConstantBufferViewOffset = 0;
};