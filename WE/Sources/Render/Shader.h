#pragma once
#include <d3d12.h>
#include <vector>
#include <unordered_map>
#include <wrl.h>
#include "Material.h"
#include "Utility/Class.h"
#include "Utility/String.h"

class FShaderManager
{
	SINGLETON(FShaderManager);
public:

private:
	void BuildShaderAndInputLayout();
	void BuildPipelineStateObject();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;
	std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>>> mPipelineStates;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mWireFramePipelineState;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> mBillboardPipelineState;
public:
	inline ID3D12PipelineState* GetPipelineStatePtr(EShadingModel ShadingModel, EBlendMode BlendMode) const
	{
		return mPipelineStates[ShadingModel][BlendMode].Get();
	}
	inline ID3D12PipelineState* GetPipelineStatePtr(std::uint32_t ShadingModel, std::uint32_t BlendMode) const
	{
		return mPipelineStates[ShadingModel][BlendMode].Get();
	}
	inline ID3D12PipelineState* GetWireFramePipelineStatePtr() const
	{
		return mWireFramePipelineState.Get();
	}
	inline ID3D12PipelineState* GetBillboardPipelineStatePtr() const
	{
		return mBillboardPipelineState.Get();
	}
};

inline FShaderManager* GetShaderManager()
{
	return FShaderManager::GetInstance();
}