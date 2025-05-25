#include "DrawRectPassCommon.hlsli"

float4 MainPS(
    float4 Position : SV_Position,
    float2 TexC : TEXCOORD
) : SV_Target
{
    return float4(gTexture[gTextureIndex].SampleLevel(gsamLinearWrap, TexC, 0).rgb, 1.0f);
}