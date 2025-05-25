#ifndef PREFILTEREDMAPPASSCOMMON_HLSLI
#define PREFILTEREDMAPPASSCOMMON_HLSLI
#include "Common.hlsli"
#include "Light.hlsli"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    float gRoughness;
    uint gSkyCubeMapIndex;
    float gResolutionOfSKyCubeMap;
    uint Pad1;
}

#endif