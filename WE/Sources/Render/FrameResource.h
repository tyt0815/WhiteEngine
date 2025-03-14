#pragma once
#include <memory>
#include <vector>
#include "Material.h"
#include "ShaderStructures.h"
#include "UploadBuffer.h"
#include "DirectX/DXUtility.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

constexpr std::uint32_t FRAME_RESOURCES_NUM = 3;
constexpr std::uint32_t PASS_COUNT = 1;


struct FObjectCBInfo
{
    FObjectConstantBuffer ObjectConstants;
    std::uint64_t DirtyFrameCount;
};

struct FFrameResource : FNoncopyable
{
public:
    FFrameResource();

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

	std::unique_ptr<TUploadBuffer<FPassConstantBuffer>> PassConstantBuffer;
	std::unique_ptr<TUploadBuffer<FObjectConstantBuffer>> ObjectConstantBuffer;
	std::unique_ptr<TUploadBuffer<FMaterialConstantBuffer>> MaterialConstantBuffer;

	UINT64 Fence = 0;
private:
    void Update();

    friend class FFrameResourceManager;
};

class FFrameResourceManager
{
    SINGLETON(FFrameResourceManager)
public:
    void Tick();
	void FlushCommandQueues();

private:
    void SetTargetFrameResource();
    void BuildRootSignature();
    void UpdatePassCB();
    void UpdateObjectCB();
    void UpdateMaterialCB();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    FFrameResource* mTargetFrameResource = nullptr;
    std::vector<std::unique_ptr<FFrameResource>> mFrameResources;
    std::uint32_t mTargetFrameResourceIndex = 0;
    TPool<FObjectCBInfo> mObjectCBInfoPool;

public:
    inline FFrameResource* GetTargetFrameResource() const
    {
        return mTargetFrameResource;
    }
    inline void SetPassCBOfTargetFrame(int i, const FPassConstantBuffer& PassCB)
    {
        mTargetFrameResource->PassConstantBuffer->CopyData(i, PassCB);
    }
    inline void SetObjectCBOfTargetFrame(int i, const FObjectConstantBuffer& ObjectCB)
    {
        mTargetFrameResource->ObjectConstantBuffer->CopyData(i, ObjectCB);
    }
    inline void SetMaterialCBOfTargetFrame(int i, const FMaterialConstantBuffer& MaterialCB)
    {
        mTargetFrameResource->MaterialConstantBuffer->CopyData(i, MaterialCB);
    }
    inline ID3D12RootSignature* GetRootSignaturePtr() const
    {
        return mRootSignature.Get();
    }
    inline std::uint64_t RegisterObjectCBInfo(const FObjectCBInfo& ObjectCBInfo)
    {
        return mObjectCBInfoPool.Register(ObjectCBInfo);
    }
    inline void RemoveObjectCBInfo(std::uint64_t Id)
    {
        mObjectCBInfoPool.Remove(Id);
    }
    inline void SetObjectCBInfo(std::uint64_t Id, const FObjectCBInfo& ObjectCBInfo)
    {
        mObjectCBInfoPool.SetItem(Id, ObjectCBInfo);
    }
};

inline FFrameResourceManager* GetFrameResourceManager()
{
    return FFrameResourceManager::GetInstance();
}