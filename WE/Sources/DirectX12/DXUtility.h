#pragma once

#include "DXHeaders.h"
#include <string>

class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

#ifndef THROWIFFAILED
#define THROWIFFAILED(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

namespace UDXUtility
{
    UINT CalcConstantBufferByteSize(UINT Size);
    D3D12_HEAP_PROPERTIES CreateHeapProperties(
        D3D12_HEAP_TYPE Type,
        D3D12_CPU_PAGE_PROPERTY CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL MemoryPool = D3D12_MEMORY_POOL_UNKNOWN,
        UINT CreationNodeMask = 1,
        UINT VisibleNodeMask = 1
    );

    D3D12_RESOURCE_DESC CreateBufferDesc(
        UINT64 Width,
        D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE,
        UINT64 Alignment = 0
    );
    D3D12_RASTERIZER_DESC CreateRasterizerDesc(
        D3D12_FILL_MODE FillMode = D3D12_FILL_MODE_SOLID,
        D3D12_CULL_MODE CullMode = D3D12_CULL_MODE_BACK,
        BOOL FrontCounterClockwise = FALSE,
        INT DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
        FLOAT DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
        FLOAT SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
        BOOL DepthClipEnable = TRUE,
        BOOL MultisampleEnable = FALSE,
        BOOL AntialiasedLineEnable = FALSE,
        UINT ForcedSampleCount = 0,
        D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
    );
    D3D12_RENDER_TARGET_BLEND_DESC CreateRenderTargetBlendDesc(
        BOOL BlendEnable = FALSE,
        BOOL LogicOpEnable = FALSE,
        D3D12_BLEND SrcBlend = D3D12_BLEND_ONE,
        D3D12_BLEND DestBlend = D3D12_BLEND_ZERO,
        D3D12_BLEND_OP BlendOp = D3D12_BLEND_OP_ADD,
        D3D12_BLEND SrcBlendAlpha = D3D12_BLEND_ONE,
        D3D12_BLEND DestBlendAlpha = D3D12_BLEND_ZERO,
        D3D12_BLEND_OP BlendOpAlpha = D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP LogicOp = D3D12_LOGIC_OP_NOOP,
        UINT8 RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
    );
    D3D12_BLEND_DESC CreateBlendDesc(
        BOOL AlphaToCoverageEnable = FALSE,
        BOOL IndependentBlendEnable = FALSE,
        D3D12_RENDER_TARGET_BLEND_DESC RenderTargetBlendDesc = CreateRenderTargetBlendDesc()
    );
    D3D12_DEPTH_STENCILOP_DESC CreateDepthStencilOPDesc(
        D3D12_STENCIL_OP StencilFailOp = D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP StencilPassOp = D3D12_STENCIL_OP_KEEP,
        D3D12_COMPARISON_FUNC StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
    );
    D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc(
        BOOL DepthEnable = TRUE,
        D3D12_DEPTH_WRITE_MASK DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
        D3D12_COMPARISON_FUNC DepthFunc = D3D12_COMPARISON_FUNC_LESS,
        BOOL StencilEnable = FALSE,
        UINT8 StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
        UINT8 StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
        D3D12_DEPTH_STENCILOP_DESC FrontFace = CreateDepthStencilOPDesc(),
        D3D12_DEPTH_STENCILOP_DESC BackFace = CreateDepthStencilOPDesc()
    );
    ComPtr<ID3D12Resource> CreateDefaultBuffer(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* cmdList,
        const void* initData,
        UINT64 byteSize,
        ComPtr<ID3D12Resource>& uploadBuffer
    );
};