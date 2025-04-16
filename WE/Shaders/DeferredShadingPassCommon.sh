#ifndef DEFERREDSHADINGPASSCOMMON_SH
#define DEFERREDSHADINGPASSCOMMON_SH
#include "Common.sh"
#include "Light.sh"

StructuredBuffer<FDirectionalLight> gDirectionalLights : register(t0, space3);

DECLARE_PASS_CB(b0);
DECLARE_LIGHTINFO_CB(b1);

cbuffer GBufferInfo : register(b2)
{
    uint gGBufferATextureIndex;
    uint gGBufferBTextureIndex;
    uint gGBufferCTextureIndex;
    uint gDepthTextureIndex;
}

DECLARE_SHADOWMAP_CB(b3);

#endif