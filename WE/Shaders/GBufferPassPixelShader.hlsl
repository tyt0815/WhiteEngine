#include "GBufferPassCommon.hlsli"

void MainPS(
    in float4 ScreenPosition : SV_Position,
    in float3 WorldNormal : NORMAL,
    in float2 TexC : TEXCOORD,
    in float3 WorldTangent : TANGENT,
    out float4 GBufferA : SV_Target0,
    out float4 GBufferB : SV_Target1,
    out float4 GBufferC : SV_Target2
)
{
    FMaterialSB MaterialData = gMaterialData[gMaterialIndex];
    
    float3 NormalMapSample = gTexture[MaterialData.NormalTextureIndex].Sample(gsamLinearWrap, TexC).rgb;
    WorldNormal = NormalSampleToWorldSpace(NormalMapSample, normalize(WorldNormal), WorldTangent);
    GBufferA = float4(WorldNormal, 1.0f);
    
    float Specular = 0.04f;
    float Roughness = gTexture[MaterialData.RoughneesTextureIndex].Sample(gsamLinearWrap, TexC).r;
    float Metallic = gTexture[MaterialData.MetallicTextureIndex].Sample(gsamLinearWrap, TexC).r;
    GBufferB = float4(Specular, Roughness, Metallic, 1.0f);
    
    float3 Albedo = gTexture[MaterialData.AlbedoTextureIndex].Sample(gsamLinearWrap, TexC).rgb;
    GBufferC = float4(Albedo, 1.0f);
}