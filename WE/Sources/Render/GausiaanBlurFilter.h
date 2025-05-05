#pragma once

#include <d3d12.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "DirectX/Resource.h"
#include "Utility/Class.h"

constexpr int BLUR_RADIUS_MAX = 5;

class FGaussianBlurFilter final : FNoncopyable
{
public:
	FGaussianBlurFilter(ID3D12Device* Device);
	FGaussianBlurFilter() = delete;
	void Execute(ID3D12GraphicsCommandList* CommandList, FResource* InputTexture, int BlurCount);
	void UpdateConstantBuffers(float Sigma);

private:
	void BuildRootSignature();
	void BuildShaders();
	void BuildPipelineStates(ID3D12Device* Device);
	std::unique_ptr<FResource> mBlurMap;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> mHorizontalComputeShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mVerticalComputeShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mHorizontalPipelineState;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mVerticalPipelineState;
	std::vector<float> mWeights;
	int mBlurRadius;
};