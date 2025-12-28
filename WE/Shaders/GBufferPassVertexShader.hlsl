#include "GBufferPassCommon.hlsli"

void MainVS(
    in FVertexInput VIn,
    out float4 ClipPosition : SV_Position,
    out float3 WorldNormal : NORMAL,
    out float2 TexC : TEXCOORD,
    out float3 WorldTangent : TANGENT
)
{
    FMaterialSB MaterialInfo = gMaterialData[gMaterialIndex];
    float4 PosW = mul(float4(VIn.LocalPosition, 1.0f), gWorld);
    
    ClipPosition = mul(PosW, gViewProj);
    WorldNormal = mul(VIn.LocalNormal, (float3x3) gInvTransposeWorld);
    TexC = mul(float4(VIn.TexC, 0.0f, 1.0f), MaterialInfo.MatTransform).xy;
    WorldTangent = mul(VIn.TangentU.xyz, (float3x3) gInvTransposeWorld);
}