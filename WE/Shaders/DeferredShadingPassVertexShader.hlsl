#include "DeferredShadingPassCommon.hlsli"

void MainVS(
    float2 InPos : POSITION,
    float2 InTexC : TEXC,
    out float4 OutPosH : SV_Position,
    out float2 OutTexC : TEXCOORD
)
{
    DrawRect(InPos, InTexC, OutPosH, OutTexC);
}