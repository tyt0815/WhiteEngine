#include "PrecomputationTransmittanceLUTPassCommon.hlsli"

float3 MainPS(
    float4 Position : SV_Position,
    float2 TexC : TEXCOORD
) : SV_Target
{
    float r;    
    float mu;   
    GetRMuFromTransmittanceTextureUv(TexC, r, mu);
    return ComputeTransmittanceToTopAtmosphereBoundary(r, mu);
}