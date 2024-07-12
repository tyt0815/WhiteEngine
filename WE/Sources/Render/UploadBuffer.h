#pragma once

#include "DXHeaders.h"
#include "Utility/Class.h"
#include "Utility/Math.h"

UINT CalcConstantBufferByteSize(UINT Size);

template<typename T>
class FUploadBuffer
{
public:
    FUploadBuffer(ID3D12Device* Device, UINT ElementCount, bool InbConstantBuffer) :
        bConstantBuffer(InbConstantBuffer)
    {
        ElementByteSize = sizeof(T);

        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (InbConstantBuffer)
            ElementByteSize = CalcConstantBufferByteSize(sizeof(T));

        D3D12_HEAP_PROPERTIES HeapProperties;
        HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProperties.CreationNodeMask = 1;
        HeapProperties.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC ResourceDesc;
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Alignment = 0;
        ResourceDesc.Width = ElementByteSize * ElementCount;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        THROWIFFAILED(Device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&UploadBuffer)));

        THROWIFFAILED(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }

    FUploadBuffer(const FUploadBuffer& rhs) = delete;
    FUploadBuffer& operator=(const FUploadBuffer& rhs) = delete;
    ~FUploadBuffer()
    {
        if (UploadBuffer != nullptr)
            UploadBuffer->Unmap(0, nullptr);

        MappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return UploadBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&MappedData[elementIndex * ElementByteSize], &data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer;
    BYTE* MappedData = nullptr;

    UINT ElementByteSize = 0;
    bool bConstantBuffer = false;
};

struct FWVPConstantBuffer
{
    DirectX::XMFLOAT4X4 WorldViewProjectMatrix = UMath::IdentityMatrix();
};