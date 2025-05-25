#include "BRDFLUTPassCommon.hlsli"
#include "Light.hlsli"

float4 MainPS(
    float4 Position : SV_Position,
    float2 TexC : TEXCOORD
) : SV_Target
{
    return float4(IntegrateBRDF(TexC.x, TexC.y), 0.0f, 1.0f);
}