#include "ShadowMapPassCommon.hlsli"

void MainVS(
    in FVertexInput Vin,
    out float4 ClipPosition : SV_Position
)
{
    FDirectionalLight DirLight = gDirectionalLights[gDirLightIndex];
    float4 WorldPosition = mul(float4(Vin.LocalPosition, 1.0f), gWorld);
    
    ClipPosition = mul(WorldPosition, DirLight.LightViewProj);
};