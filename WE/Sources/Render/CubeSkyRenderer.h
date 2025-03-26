#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>
#include "UploadBuffer.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "CubeRenderTarget.h"

class FTexture;
class FCubeRenderTarget;

class FCubeSkyIrradianceMapRenderer : FNoncopyable
{
	struct FConstantBuffers
	{
		XMFLOAT4X4 ViewProj;
		UINT SkyCubeMapIndex;
		UINT Pad1;
		UINT Pad2;
		UINT Pad3;
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
	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mTempCB;
	FCubeRenderTarget* mCubeRenderTarget;
	FTexture* mSkyTextureCube = nullptr;
};

class FCubeSkyRenderer : FNoncopyable
{
	struct FConstantBuffers
	{
		XMFLOAT4X4 gViewProj;
		XMFLOAT3 gEyePosW;
		UINT SkyCubeMapIndex;
	};
public:
	FCubeSkyRenderer(std::string SkyCubeMapName);
	FCubeSkyRenderer() = delete;
	virtual void Render(ID3D12GraphicsCommandList* CommandList);

private:
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	void BuildCBs();
	void BuildRootSignature();
	void CraeteIrradianceMap();
	void UpdateCBs();
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	std::unique_ptr<FCubeRenderTarget> mSkyIrradianceMapRenderTarget;
	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mCB;
	FTexture* mSkyTextureCube = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::string mSkyIrradianceMapName;
	UINT SkyIrradianceCubeMapSRVHeapIndex;

public:
	inline UINT GetSkyIrradianceCubeMapSRVHeapIndex() const
	{
		return SkyIrradianceCubeMapSRVHeapIndex;
	}
};