#pragma once
#include <memory>
#include <vector>
#include "Material.h"
#include "UploadBuffer.h"
#include "DirectX/DXUtility.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

class FFrameResourceManager
{
    SINGLETON(FFrameResourceManager)
public:

private:
    void BuildRootSignature();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    std::uint32_t mTargetFrameResourceIndex = 0;

public:
    inline ID3D12RootSignature* GetRootSignaturePtr() const
    {
        return mRootSignature.Get();
    }
};

inline FFrameResourceManager* GetFrameResourceManager()
{
    return FFrameResourceManager::GetInstance();
}