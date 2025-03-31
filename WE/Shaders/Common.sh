#ifndef COMMON_SH
#define COMMON_SH
#include "Math.sh"    

Texture2D gTexture[1024] : register(t0, space0);
TextureCube gTextureCube[1024] : register(t0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

void DrawSphere(in float3 InPosL, in float4x4 ViewProj, out float3 OutPosL, out float4 OutPosH)
{
    OutPosL = InPosL;
    OutPosH = mul(float4(InPosL, 1.0f), ViewProj);
}

void DrawRect(in float2 InPosition, in float2 InTexC, out float4 OutPosition, out float2 OutTexC)
{
    OutPosition = float4(InPosition, 0.0f, 1.0f);
    OutTexC = InTexC;
}

#endif