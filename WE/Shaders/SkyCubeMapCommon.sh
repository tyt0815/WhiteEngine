#ifndef SKYCUBEMAPCOMMON_SH
#define SKYCUBEMAPCOMMON_SH
#include "Common.sh"

TextureCube gTextureCube : register(t0);
cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    float3 gEyePosW;
    uint Pad1;
}

#endif