#ifndef COMMON_SH
#define COMMON_SH
#include "Math.sh"    

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

#endif