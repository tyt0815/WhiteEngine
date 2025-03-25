#ifndef SKYCUBEMAPCOMMON_SH
#define SKYCUBEMAPCOMMON_SH
#include "Common.sh"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    float3 gEyePosW;
    uint gSkyCubeMapIndex;
}

#endif