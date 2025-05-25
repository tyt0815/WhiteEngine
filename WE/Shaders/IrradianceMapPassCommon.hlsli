#ifndef IRRADIANCEMAPPASSCOMMON_HLSLI
#define IRRADIANCEMAPPASSCOMMON_HLSLI
#include "Common.hlsli"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    uint gSkyCubeMapIndex;
    uint Pad1;
    uint Pad2;
    uint Pad3;
}

#endif