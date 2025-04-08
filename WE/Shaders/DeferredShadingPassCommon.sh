#ifndef DEFERREDSHADINGPASSCOMMON_SH
#define DEFERREDSHADINGPASSCOMMON_SH
#include "Common.sh"
#include "Light.sh"

StructuredBuffer<FDirectionalLight> gDirectionalLights : register(t0, space3);

DECLARE_PASS_CB(b0);

cbuffer GBufferInfo : register(b1)
{
    uint gGBufferATextureIndex;
    uint gGBufferBTextureIndex;
    uint gGBufferCTextureIndex;
    uint gDepthTextureIndex;
}

#endif