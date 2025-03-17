#pragma once
#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include "Utility/Class.h"

class FCubeSkyRenderer
{
	SINGLETON(FCubeSkyRenderer);
public:
	virtual void Render(ID3D12GraphicsCommandList* CommandList);

private:
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
};