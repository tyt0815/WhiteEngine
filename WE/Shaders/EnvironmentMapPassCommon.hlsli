#ifndef ENVIRONMENTMAPPASSCOMMON_HLSLI
#define ENVIRONMENTMAPPASSCOMMON_HLSLI
#include "Common.hlsli"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    float3 gEyePosW;
    uint gSkyCubeMapIndex;
}

#endif