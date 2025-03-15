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


struct FMeshCBInfo
{
    FMeshConstantBuffer MeshCB;
    std::uint64_t DirtyFrameCount;
};

struct FSubmeshCBInfo
{
	FSubmeshConstantBuffer SubmeshCB;
	std::uint64_t DirtyFrameCount;
};

struct FFrameResource : FNoncopyable
{
public:
    FFrameResource();

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

	std::unique_ptr<TUploadBuffer<FPassConstantBuffer>> PassConstantBuffer;
	std::unique_ptr<TUploadBuffer<FMeshConstantBuffer>> MeshConstantBuffer;
	std::unique_ptr<TUploadBuffer<FSubmeshConstantBuffer>> SubmeshConstantBuffer;
	std::unique_ptr<TUploadBuffer<FMaterialStructuredBuffer>> MaterialConstantBuffer;

	UINT64 Fence = 0;
private:

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
    void UpdateMeshCB();
    void UpdateSubmeshCB();
    void UpdateMaterialCB();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    FFrameResource* mTargetFrameResource = nullptr;
    std::vector<std::unique_ptr<FFrameResource>> mFrameResources;
    std::uint32_t mTargetFrameResourceIndex = 0;
    TPool<FMeshCBInfo> mMeshCBInfoPool;
	TPool<FSubmeshCBInfo> mSubmeshCBInfoPool;

public:
    inline FFrameResource* GetTargetFrameResource() const
    {
        return mTargetFrameResource;
    }
    inline void SetPassCBOfTargetFrame(int i, const FPassConstantBuffer& PassCB)
    {
        mTargetFrameResource->PassConstantBuffer->CopyData(i, PassCB);
    }
    inline void SetObjectCBOfTargetFrame(int i, const FMeshConstantBuffer& ObjectCB)
    {
        mTargetFrameResource->MeshConstantBuffer->CopyData(i, ObjectCB);
    }
    inline void SetMaterialCBOfTargetFrame(int i, const FMaterialStructuredBuffer& MaterialCB)
    {
        mTargetFrameResource->MaterialConstantBuffer->CopyData(i, MaterialCB);
    }
    inline ID3D12RootSignature* GetRootSignaturePtr() const
    {
        return mRootSignature.Get();
    }
    inline std::uint64_t RegisterMeshCBInfo(const FMeshCBInfo& ObjectCBInfo)
    {
        return mMeshCBInfoPool.Register(ObjectCBInfo);
    }
    inline void RemoveMeshCBInfo(std::uint64_t Id)
    {
        mMeshCBInfoPool.Remove(Id);
    }
    inline void SetMeshCBInfo(std::uint64_t Id, const FMeshCBInfo& ObjectCBInfo)
    {
        mMeshCBInfoPool.SetItem(Id, ObjectCBInfo);
    }
	inline std::uint64_t RegisterSubmeshCBInfo(const FSubmeshCBInfo& SubmeshCBInfo)
	{
		return mSubmeshCBInfoPool.Register(SubmeshCBInfo);
	}
	inline void RemoveSubmeshCBInfo(std::uint64_t Id)
	{
		mSubmeshCBInfoPool.Remove(Id);
	}
	inline void SetSubmeshCBInfo(std::uint64_t Id, const FSubmeshCBInfo& SubmeshCBInfo)
	{
		mSubmeshCBInfoPool.SetItem(Id, SubmeshCBInfo);
	}
};

inline FFrameResourceManager* GetFrameResourceManager()
{
    return FFrameResourceManager::GetInstance();
}