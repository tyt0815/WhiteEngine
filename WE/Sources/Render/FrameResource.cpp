#include "FrameResource.h"

const int NumFrameResources = 3;

FFrameResource::FFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount)
{
    ThrowIfFailed(Device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(CommandAllocator.GetAddressOf())));

    PassConstantBuffer = std::make_unique<TUploadBuffer<FPassConstants>>(Device, PassCount, true);
    ObjectConstantBuffer= std::make_unique<TUploadBuffer<FObjectConstants>>(Device, ObjectCount, true);
    MaterialConstantBuffer = std::make_unique<TUploadBuffer<FMaterialConstants>>(Device, MaterialCount, true);
}
