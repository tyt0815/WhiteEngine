#include "DXUtility.h"

#include <comdef.h>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

std::wstring DxException::ToString() const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    std::wstring msg = err.ErrorMessage();

    return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

UINT UDXUtility::CalcConstantBufferByteSize(UINT Size)
{
    // Constant buffers must be a multiple of the minimum hardware
    // allocation size (usually 256 bytes).  So round up to nearest
    // multiple of 256.  We do this by adding 255 and then masking off
    // the lower 2 bytes which store all bits < 256.
    // Example: Suppose byteSize = 300.
    // (300 + 255) & ~255
    // 555 & ~255
    // 0x022B & ~0x00ff
    // 0x022B & 0xff00
    // 0x0200
    // 512
    return (Size + 255) & ~255;
}

D3D12_HEAP_PROPERTIES UDXUtility::CreateHeapProperties(
    D3D12_HEAP_TYPE Type,
    D3D12_CPU_PAGE_PROPERTY CPUPageProperty,
    D3D12_MEMORY_POOL MemoryPool,
    UINT CreationNodeMask,
    UINT VisibleNodeMask
)
{
    D3D12_HEAP_PROPERTIES HeapProperties;
    HeapProperties.Type = Type;
    HeapProperties.CPUPageProperty = CPUPageProperty;
    HeapProperties.MemoryPoolPreference = MemoryPool;
    HeapProperties.CreationNodeMask = CreationNodeMask;
    HeapProperties.VisibleNodeMask = VisibleNodeMask;
    return HeapProperties;
}

D3D12_RESOURCE_DESC UDXUtility::CreateBufferDesc(
    UINT64 Width,
    D3D12_RESOURCE_FLAGS Flags,
    UINT64 Alignment
)
{
    D3D12_RESOURCE_DESC BufferDesc;
    BufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    BufferDesc.Alignment = 0;
    BufferDesc.Width = Width;
    BufferDesc.Height = 1;
    BufferDesc.DepthOrArraySize = 1;
    BufferDesc.MipLevels = 1;
    BufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.SampleDesc.Quality = 0;
    BufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    BufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return BufferDesc;
}

D3D12_RASTERIZER_DESC UDXUtility::CreateRasterizerDesc(
    D3D12_FILL_MODE FillMode,
    D3D12_CULL_MODE CullMode,
    BOOL FrontCounterClockwise, 
    INT DepthBias, 
    FLOAT DepthBiasClamp, 
    FLOAT SlopeScaledDepthBias, 
    BOOL DepthClipEnable, 
    BOOL MultisampleEnable, 
    BOOL AntialiasedLineEnable, 
    UINT ForcedSampleCount, 
    D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster
)
{
    D3D12_RASTERIZER_DESC RasterizerDesc;
    RasterizerDesc.FillMode = FillMode;
    RasterizerDesc.CullMode = CullMode;
    RasterizerDesc.FrontCounterClockwise = FrontCounterClockwise;
    RasterizerDesc.DepthBias = DepthBias;
    RasterizerDesc.DepthBiasClamp = DepthBiasClamp;
    RasterizerDesc.SlopeScaledDepthBias = SlopeScaledDepthBias;
    RasterizerDesc.DepthClipEnable = DepthClipEnable;
    RasterizerDesc.MultisampleEnable = MultisampleEnable;
    RasterizerDesc.AntialiasedLineEnable = AntialiasedLineEnable;
    RasterizerDesc.ForcedSampleCount = ForcedSampleCount;
    RasterizerDesc.ConservativeRaster = ConservativeRaster;
    return RasterizerDesc;
}

D3D12_RENDER_TARGET_BLEND_DESC UDXUtility::CreateRenderTargetBlendDesc(
    BOOL BlendEnable,
    BOOL LogicOpEnable,
    D3D12_BLEND SrcBlend,
    D3D12_BLEND DestBlend,
    D3D12_BLEND_OP BlendOp,
    D3D12_BLEND SrcBlendAlpha,
    D3D12_BLEND DestBlendAlpha,
    D3D12_BLEND_OP BlendOpAlpha,
    D3D12_LOGIC_OP LogicOp,
    UINT8 RenderTargetWriteMask
)
{
    D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc;
    RenderTargetBlendDesc.BlendEnable = BlendEnable;
    RenderTargetBlendDesc.LogicOpEnable = LogicOpEnable;
    RenderTargetBlendDesc.SrcBlend = SrcBlend;
    RenderTargetBlendDesc.DestBlend = DestBlend;
    RenderTargetBlendDesc.BlendOp = BlendOp;
    RenderTargetBlendDesc.SrcBlendAlpha = SrcBlendAlpha;
    RenderTargetBlendDesc.DestBlendAlpha = DestBlendAlpha;
    RenderTargetBlendDesc.BlendOpAlpha = BlendOpAlpha;
    RenderTargetBlendDesc.LogicOp = LogicOp;
    RenderTargetBlendDesc.RenderTargetWriteMask = RenderTargetWriteMask;
    return RenderTargetBlendDesc;
}

D3D12_BLEND_DESC UDXUtility::CreateBlendDesc(
    BOOL AlphaToCoverageEnable,
    BOOL IndependentBlendEnable,
    D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc
)
{
    D3D12_BLEND_DESC BlendDesc;
    BlendDesc.AlphaToCoverageEnable = AlphaToCoverageEnable;
    BlendDesc.IndependentBlendEnable = IndependentBlendEnable;
    for (auto& RT : BlendDesc.RenderTarget)
    {
        RT = RenderTargetBlendDesc;
    }
    return BlendDesc;
}

D3D12_DEPTH_STENCILOP_DESC UDXUtility::CreateDepthStencilOPDesc(D3D12_STENCIL_OP StencilFailOp, D3D12_STENCIL_OP StencilDepthFailOp, D3D12_STENCIL_OP StencilPassOp, D3D12_COMPARISON_FUNC StencilFunc)
{
    D3D12_DEPTH_STENCILOP_DESC DepthStencilOPDesc;
    DepthStencilOPDesc.StencilFailOp = StencilFailOp;
    DepthStencilOPDesc.StencilDepthFailOp = StencilDepthFailOp;
    DepthStencilOPDesc.StencilPassOp = StencilPassOp;
    DepthStencilOPDesc.StencilFunc = StencilFunc;
    return DepthStencilOPDesc;
}

D3D12_DEPTH_STENCIL_DESC UDXUtility::CreateDepthStencilDesc(
    BOOL DepthEnable,
    D3D12_DEPTH_WRITE_MASK DepthWriteMask,
    D3D12_COMPARISON_FUNC DepthFunc,
    BOOL StencilEnable,
    UINT8 StencilReadMask,
    UINT8 StencilWriteMask,
    D3D12_DEPTH_STENCILOP_DESC FrontFace,
    D3D12_DEPTH_STENCILOP_DESC BackFace
)
{
    D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;
    DepthStencilDesc.DepthEnable = DepthEnable;
    DepthStencilDesc.DepthWriteMask = DepthWriteMask;
    DepthStencilDesc.DepthFunc = DepthFunc;
    DepthStencilDesc.StencilEnable = StencilEnable;
    DepthStencilDesc.StencilReadMask = StencilReadMask;
    DepthStencilDesc.StencilWriteMask = StencilWriteMask;
    DepthStencilDesc.FrontFace = FrontFace;
    DepthStencilDesc.BackFace = BackFace;
    return DepthStencilDesc;
}

ComPtr<ID3D12Resource> UDXUtility::CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    ComPtr<ID3D12Resource>& uploadBuffer
)
{
    ComPtr<ID3D12Resource> defaultBuffer;
    D3D12_HEAP_PROPERTIES HeapProperties = CreateHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC BufferDesc = CreateBufferDesc(byteSize);
    // Create the actual default buffer resource.
    THROWIFFAILED(
        device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &BufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(defaultBuffer.GetAddressOf())
        )
    );

    // In order to copy CPU memory data into our default buffer, we need to create
    // an intermediate upload heap. 
    HeapProperties = CreateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    THROWIFFAILED(
        device->CreateCommittedResource(
            &HeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &BufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())
        )
    );


    // Describe the data we want to copy into the default buffer.
    
    //D3D12_SUBRESOURCE_DATA subResourceData = {};
    //subResourceData.pData = initData;
    //subResourceData.RowPitch = byteSize;
    //subResourceData.SlicePitch = subResourceData.RowPitch;

    //// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    //// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    //// the intermediate upload heap data will be copied to mBuffer.
    //cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    //    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
    //UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
    //cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    //    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

    // Note: uploadBuffer has to be kept alive after the above function calls because
    // the command list has not been executed yet that performs the actual copy.
    // The caller can Release the uploadBuffer after it knows the copy has been executed.


    return defaultBuffer;
}