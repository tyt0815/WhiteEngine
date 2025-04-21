#ifndef IRRADIANCEMAPPASSCOMMON_SH
#define IRRADIANCEMAPPASSCOMMON_SH
#include "Common.sh"

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gViewProj;
    uint gSkyCubeMapIndex;
    uint Pad1;
    uint Pad2;
    uint Pad3;
}

#endif