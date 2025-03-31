#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>
#include "UploadBuffer.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "RenderTarget.h"
#include "CubeRenderTarget.h"

class FTexture;
class FRenderTarget;
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
	void BuildPipelineState();
	void BuildShaders();
	void BuildRootSignature();
	void BuildCB();
	void UpdateCBs();
	void Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mTempCB;
	FCubeRenderTarget* mCubeRenderTarget;
	FTexture* mSkyTextureCube = nullptr;
};

class FPreFilteredSkyCubeMapRenderer : FNoncopyable
{
	struct FConstantBuffers
	{
		XMFLOAT4X4 ViewProj;
		float Roughness;
		UINT SkyCubeMapIndex;
		float ResolutionOfSkyCubeMap;
		UINT Pad1;
	};

public:
	FPreFilteredSkyCubeMapRenderer(FCubeRenderTarget* CubeRenderTarget);
	FPreFilteredSkyCubeMapRenderer() = delete;
	void Render(FTexture* SkyTextureCube);

private:
	void BuildCB();
	void BuildRootSignature();
	void BuildShaders();
	void BuildPipelineState();
	void UpdateCBs();
	void Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature; 
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mCB;
	FCubeRenderTarget* mCubeRenderTarget;
	FTexture* mSkyTextureCube = nullptr;
};

class FIndirectSpecularIntegralRenderer : FNoncopyable
{
	struct FConstantBuffers
	{

	};
public:
	FIndirectSpecularIntegralRenderer(FRenderTarget* RenderTarget);
	FIndirectSpecularIntegralRenderer() = delete;
	void Render(FTexture* SkyTextureCube);

private:
	void BuildCB();
	void BuildRootSignature();
	void BuildShaders();
	void BuildPipelineState();
	void UpdateCBs();
	void Internal_Render(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;

	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mCB;
	FRenderTarget* mRenderTarget;
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
	void CreatePreFilteredSkyCubeMap();
	void CreateIndirectSpecularIntegral();
	void UpdateCBs();
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipelineState;
	std::unique_ptr<FCubeRenderTarget> mSkyIrradianceMapRenderTarget;
	std::unique_ptr<FCubeRenderTarget> mPreFilteredSkyCubeMapRenderTarget;
	std::unique_ptr<FRenderTarget> mIndirectSpecularIntegralRenderTarget;
	std::unique_ptr<TUploadBuffer<FConstantBuffers>> mCB;
	FTexture* mSkyTextureCube = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	std::string mSkyIrradianceMapName;
	std::string mPreFilteredSkyCubeMapName;
	std::string mIndirectSpecularIntegralTextureName;
	UINT SkyIrradianceCubeMapSRVHeapIndex;
	UINT mPreFilteredSkyCubeMapSRVHeapIndex;
	UINT mIndirectSpecularIntegralTextureSRVHeapIndex;
	

public:
	inline UINT GetSkyIrradianceCubeMapSRVHeapIndex() const
	{
		return SkyIrradianceCubeMapSRVHeapIndex;
	}
	inline UINT GetPreFilteredSkyCubeMapSRVHeapIndex() const
	{
		return mPreFilteredSkyCubeMapSRVHeapIndex;
	}
	inline UINT GetIndirectSpecularIntegralTextureSRVHeapIndex() const
	{
		return mIndirectSpecularIntegralTextureSRVHeapIndex;
	}
};