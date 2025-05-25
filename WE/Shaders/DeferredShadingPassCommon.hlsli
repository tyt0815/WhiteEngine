#ifndef DEFERREDSHADINGPASSCOMMON_HLSLI
#define DEFERREDSHADINGPASSCOMMON_HLSLI
#include "Common.hlsli"
#include "Light.hlsli"

DECLARE_LIGHT_SB(t0, space3);
DECLARE_PASS_CB(b0);
DECLARE_LIGHTINFO_CB(b1);

cbuffer PassConstantBuffers : register(b2)
{
    uint gGBufferATextureIndex;
    uint gGBufferBTextureIndex;
    uint gGBufferCTextureIndex;
    uint gDepthTextureIndex;
    
    uint gIrradianceMapIndex;
    uint gPrefilteredMapIndex;
    uint gBRDFLUTIndex;
    uint PCBPad1;
}

#endif