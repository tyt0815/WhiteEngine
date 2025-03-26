#ifndef PREFILTEREDSKYCUBEMAP_SH
#define PREFILTEREDSKYCUBEMAP_SH
#include "Common.sh"
#include "Light.sh"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    float gRoughness;
    uint gSkyCubeMapIndex;
    float gResolutionOfSKyCubeMap;
    uint Pad1;
}

#endif