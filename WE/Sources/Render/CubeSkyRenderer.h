#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>
#include "Utility/Class.h"

class FCubeRenderTarget;
class FTexture;

class FCubeSkyRenderer
{
	SINGLETON(FCubeSkyRenderer);
public:
	virtual void Render(ID3D12GraphicsCommandList* CommandList);

private:
	void BuildDiffuseCubeMap();
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	std::unique_ptr<FCubeRenderTarget> mDiffuseCubeMap = nullptr;
	ID3D12Device* mDevice = nullptr;
	FTexture* mCubeMap = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
};