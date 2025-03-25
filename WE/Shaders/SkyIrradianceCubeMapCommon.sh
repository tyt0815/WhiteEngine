#ifndef SKYIRRADIANCECUBEMAPCOMMON_SH
#define SKYIRRADIANCECUBEMAPCOMMON_SH
#include "Common.sh"

TextureCube gTextureCube : register(t0);
Texture2D gTexture : register(t1);

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
}

#endif