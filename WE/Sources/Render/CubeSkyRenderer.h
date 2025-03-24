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

void DrawSphere(ID3D12GraphicsCommandList* CommandList);

class FCubeSkyIrradianceMapRenderer
{
	struct FTempCB
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Proj;
		XMFLOAT4X4 ViewProj;
	};
public:
	FCubeSkyIrradianceMapRenderer(FCubeRenderTarget* CubeRenderTarget);
	FCubeSkyIrradianceMapRenderer() = delete;

	void Render(FTexture* SkyTextureCube);
private:
	void Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	std::unique_ptr<TUploadBuffer<FTempCB>> mTempCB;
	FCubeRenderTarget* mCubeRenderTarget;
	FTexture* mSkyTextureCube = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
};

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
	std::unique_ptr<FCubeRenderTarget> mSkyIrradianceMapRenderTarget;
	FTexture* mSkyTextureCube = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::string mSkyIrradianceMapName;
};