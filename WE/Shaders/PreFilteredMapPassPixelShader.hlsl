#include "PreFilteredMapPassCommon.hlsli"

#define SAMPLE_COUNT 1024u

float4 MainPS(
    float3 PosL : POSITION0,
    float4 PosH : SV_Position
) : SV_Target
{
    float3 N = normalize(PosL);
    float3 R = N;
    float3 V = R;
    
    float TotalWeight = 0.0f;
    float3 PrefillteredColor = (float3) 0.0f;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, gRoughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        
        float NdotL = max(dot(N, L), 0.0f);
        if(NdotL > 0.0f)
        {
            float D = DistributionGGX(N, H, gRoughness);
            float pdf = (D * dot(N, H) / (4.f * dot(H, V))) + 0.0001;
            float saTexel = 4.f * PI / (6.f * gResolutionOfSKyCubeMap * gResolutionOfSKyCubeMap);
            float saSample = 1.f / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float MipLevel = gRoughness == 0.f ? 0.f : 0.5f * log2(saSample / saTexel);
            
            PrefillteredColor += gTextureCube[gSkyCubeMapIndex].SampleLevel(gsamLinearWrap, L, MipLevel).rgb * NdotL;
            TotalWeight += NdotL;
        }
    }
    PrefillteredColor = PrefillteredColor / TotalWeight;
    return float4(PrefillteredColor, 1.0f);
}