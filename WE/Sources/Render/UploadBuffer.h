#pragma once

#include "DirectX12/Direct3D12Util.h"

enum class EUploadBufferType
{
    EUBT_ConstantBuffer
};

template<typename T>
class FUploadBuffer
{
public:
    FUploadBuffer(ID3D12Device* Device, UINT ElementCount, EUploadBufferType UploadBufferType) :
        Type(UploadBufferType)
    {
        ElementByteSize = sizeof(T);

        // Constant buffer elements need to be multiples of 256 bytes.
        // This is because the hardware can only view constant data 
        // at m*256 byte offsets and of n*256 byte lengths. 
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (UploadBufferType == EUploadBufferType::EUBT_ConstantBuffer)
            ElementByteSize = CalcConstantBufferByteSize(sizeof(T));

        CD3DX12_HEAP_PROPERTIES D3D12HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC D3D12ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ElementByteSize * ElementCount);
        ThrowIfFailed(Device->CreateCommittedResource(
            &D3D12HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &D3D12ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&UploadBuffer)));

        ThrowIfFailed(UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&MappedData)));

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

    UINT GetByteSize() { return ElementByteSize; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer;
    BYTE* MappedData = nullptr;

    UINT ElementByteSize = 0;
    EUploadBufferType Type = EUploadBufferType::EUBT_ConstantBuffer;
};
