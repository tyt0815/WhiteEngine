#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "UploadBuffer.h"
#include "CubeRenderTarget.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "Utility/Class.h"
#include "Utility/String.h"

class FEnvironmentMapRenderer final : FNoncopyable
{
	struct FIrradianceMapPassCB
	{
		DirectX::XMFLOAT4X4 ViewProj;
		UINT SkyCubeMapIndex;
		UINT Pad1;
		UINT Pad2;
		UINT Pad3;
	};

	struct FPreFilteredMapPassCB
	{
		DirectX::XMFLOAT4X4 ViewProj;
		float Roughness;
		UINT SkyCubeMapIndex;
		float ResolutionOfSkyCubeMap;
		UINT Pad1;
	};

	struct FEnvironmentMapPassCB
	{
		DirectX::XMFLOAT4X4 ViewProj;
		DirectX::XMFLOAT3 EyePosW;
		UINT SkyCubeMapIndex;
	};

public:
	FEnvironmentMapRenderer(FTexture* TextureCube);
	void Render(
		ID3D12GraphicsCommandList* CommandList,
		D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
		D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
		D3D12_VIEWPORT Viewport
	);

private:
	void BuildRenderTargetsAndDepthStencil();

	void BuildBuffers();

	void BuildRootSignatures();
	void BuildIrradianceMapPassRootSignature();
	void BuildPreFilteredMapPassRootSignature();
	void BuildBRDFLUTPassRootSignature();
	void BuildEnvironmentMapPassRootSignature();

	void BuildShadersAndInputLayouts();
	void BuildIrradianceMapPassShaders();
	void BuildPreFilteredMapPassShaders();
	void BuildBRDFLUTPassShaders();
	void BuildEnvironmentMapPassShaders();

	void BuildPipelineStates();
	void BuildIrradianceMapPassPipelineState();
	void BuildPreFilteredMapPassPipelineState();
	void BuildBRDFLUTPassPipelineState();
	void BuildEnvironmentMapPassPipelineState();

	void PreRender(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void PreRenderIrradianceMapPass(ID3D12GraphicsCommandList* CommandList);
	void PreRenderPrefilteredMapPass(ID3D12GraphicsCommandList* CommandList);
	void PreRenderBRDFLUTPass(ID3D12GraphicsCommandList* CommandList);

	void UpdateBuffers();

	std::unique_ptr<FCubeRenderTarget> mIrradianceMapRenderTarget;
	std::unique_ptr<FCubeRenderTarget> mPrefilteredMapRenderTarget;
	std::unique_ptr<FRenderTarget> mBRDFLUTRenderTarget;
	std::unique_ptr<FDepthStencil> mDepthStencil;
	std::unique_ptr<TUploadBuffer<FIrradianceMapPassCB>> mIrradianceMapPassCB;
	std::unique_ptr<TUploadBuffer<FPreFilteredMapPassCB>> mPreFilteredMapPassCB;
	std::unique_ptr<TUploadBuffer<FEnvironmentMapPassCB>> mEnvironmentMapPassCB;
	ID3D12Device* mDevice;
	FTexture* mSkyTextureCube;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPipelineStates;


public:
	inline FCubeRenderTarget* GetIrradianceMapRenderTarget() const
	{
		return mIrradianceMapRenderTarget.get();
	}
	inline FCubeRenderTarget* GetPrefilteredMapRenderTarget() const
	{
		return mPrefilteredMapRenderTarget.get();
	}
	inline FRenderTarget* GetBRDFLUTRenderTarget() const
	{
		return mBRDFLUTRenderTarget.get();
	}
};