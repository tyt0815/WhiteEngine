#pragma once
#include <array>
#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "Material.h"
#include "Utility/Class.h"
#include "Utility/String.h"

extern const int gFrameResourcesNum;
constexpr int FRAME_RESOURCES_NUM = 3;

class FFrameResource : FNoncopyable
{
public:
    FFrameResource();
    virtual ~FFrameResource();
    void Flush();

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
    UINT64 mFenceCount;

public:
    inline ID3D12CommandAllocator* GetCommandAllocatorPtr() const
    {
        return mCommandAllocator.Get();
    }
    inline UINT64 GetFenceCount() const
    {
        return mFenceCount;
    }
    inline void SetFenceCount(UINT64 FenceCount)
    {
        mFenceCount = FenceCount;
    }
};

class FSceneRenderer : FNoncopyable
{
public:
	FSceneRenderer();
	~FSceneRenderer() = default;
    virtual void Render();
    virtual void Destroy();

protected:
    virtual void CreateFrameResources() = 0;
    virtual void BuildRootSignature() = 0;
    virtual void BuildShadersAndInputLayouts() = 0;
    virtual void BuildPipelineStates() = 0;
    virtual void UpdateFrameBuffers(FFrameResource* FrameResource) = 0;
    

    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mWireFramePipelineState;
    std::array<std::unique_ptr<FFrameResource>, FRAME_RESOURCES_NUM> mFrameResources;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;
    std::array<std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, EBM_None>, ESM_None> mPipelineStates;
    int mTargetFrameResourceIndex = 0;
    bool bWireFrame = false;

private:
    void UpdateTargetFrameResource();
    void SwitchToNextFrameResource();

public:
    inline FFrameResource* GetTargetFrameResource() const
    {
        return mFrameResources[mTargetFrameResourceIndex].get();
    }
};