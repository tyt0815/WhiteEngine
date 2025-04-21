#ifndef DEFERREDSHADINGPASSCOMMON_SH
#define DEFERREDSHADINGPASSCOMMON_SH
#include "Common.sh"
#include "Light.sh"

DECLARE_LIGHT_SB(t0, space3);
DECLARE_PASS_CB(b0);
DECLARE_LIGHTINFO_CB(b1);

cbuffer GBufferInfo : register(b2)
{
    uint gGBufferATextureIndex;
    uint gGBufferBTextureIndex;
    uint gGBufferCTextureIndex;
    uint gDepthTextureIndex;
}

#endif