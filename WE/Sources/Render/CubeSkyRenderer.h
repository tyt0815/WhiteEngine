#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>
#include "UploadBuffer.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"

class FTexture;
class FCubeRenderTarget;

class FCubeSkyRenderer
{
	struct FTempCB
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Proj;
		XMFLOAT4X4 ViewProj;
	};
	SINGLETON(FCubeSkyRenderer);
public:
	virtual void Render(ID3D12GraphicsCommandList* CommandList);

private:
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	void BuildDiffuseCubeMap();
	void BuildDescriptorHeaps();
	void BuildDepthStencilBuffer();
	void BuildViewMatrix();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void RenderDiffuseMap(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void DrawSphere(ID3D12GraphicsCommandList* CommandList);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDiffuseDepthStencilResource;
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mDiffuseMapVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mDiffuseMapPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mDiffuseCubePipelineState;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	std::unique_ptr<TUploadBuffer<FTempCB>> mTempCB;
	std::unique_ptr<FCubeRenderTarget> mDiffuseCubeRenderTarget;
	ID3D12Device* mDevice = nullptr;
	FTexture* mSkyTextureCube = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	XMFLOAT4X4 mCubeViews[6];
	DXGI_FORMAT mDiffuseTextureCubeFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};