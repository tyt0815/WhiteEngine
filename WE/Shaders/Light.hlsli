#ifndef LIGHT_HLSLI
#define LIGHT_HLSLI

#include "Math.hlsli"
#include "Material.hlsli"

#ifndef DIR_LIGHTS_NUM 
#define DIR_LIGHTS_NUM 1
#endif

#ifndef POINT_LIGHTS_NUM
#define POINT_LIGHTS_NUM 0
#endif

#ifndef SPOT_LIGHTS_NUM
#define SPOT_LIGHTS_NUM 0
#endif

#ifndef TEXTURES
#define TEXTURES
Texture2D gTexture[1024] : register(t0, space0);
TextureCube gTextureCube[1024] : register(t0, space1);
#endif

SamplerComparisonState gsamShadowMap : register(s6);

struct FDirectionalLight
{
    float4x4 LightViewProj;
    float4x4 ShadowTransform;
    float3 Direction;
    uint ShadowMapIndex;
    float3 Color;
    uint Pad2;
};

#define DECLARE_LIGHT_SB(t, s)\
StructuredBuffer<FDirectionalLight> gDirectionalLights : register(t, s);

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float Roughness)
{
    return F0 + (max((float3)(1.0f - Roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float DistributionGGX(float3 N, float3 H, float Roughness)
{
    float a = Roughness * Roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float Num = a2;
    float Denom = (NdotH2 * (a2 - 1.0) + 1.0f);
    Denom = PI * Denom * Denom;

    return Num / Denom;
}

float GeometrySchlickGGX(float NdotV, float Roughness)
{
    float r = (Roughness + 1.0);
    float k = (r * r) / 8.0;

    float Num = NdotV;
    float Denom = NdotV * (1.0 - k) + k;
	
    return Num / Denom;
}

float CalculateShadowFactor(float4 ShadowPositionH, in Texture2D ShadowMap)
{
    ShadowPositionH.xyz /= ShadowPositionH.w;
    float Depth = ShadowPositionH.z;
    
    uint Width;
    uint Height;
    uint NumMips;
    ShadowMap.GetDimensions(0, Width, Height, NumMips);
    
    float dx = 1.0f / Width;
    float PercentLit = 0.0f;
    const float2 Offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, dx), float2(0.0f, dx), float2(dx, dx)
    };
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        PercentLit += ShadowMap.SampleCmpLevelZero(gsamShadowMap, ShadowPositionH.xy + Offsets[i], Depth).r;
    }
    
    return PercentLit / 9.0f;
}

float GeometrySmith(float3 N, float3 V, float3 L, float Roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, Roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, Roughness);
	
    return ggx1 * ggx2;
}

float3 CalculateF0(float Specular, float3 Albedo, float Metallic)
{
    return lerp((float3) Specular, Albedo, Metallic);
}

float3 ComputeDirectionalLight(
    FDirectionalLight Light,
    FMaterial Material,
    float3 V,
    float3 N,
    float3 F0
)
{
    
    float3 L = normalize(-Light.Direction);
    float3 H = normalize(V + L);
    
    float3 Radiance = Light.Color;
        
    float NDF = DistributionGGX(N, H, Material.Roughness);
    float G = GeometrySmith(N, V, L, Material.Roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    float3 kS = F;
    float3 kD = (float3) 1.0f - kS;
    kD *= 1.0f - Material.Metallic;
    
    float3 Numerator = NDF * G * F;
    float Denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.00001f;
    float3 Specular = Numerator / Denominator;
    
    float NDotL = max(dot(N, L), 0.0f);
    
    return (kD * Material.Albedo / PI + Specular) * Radiance * NDotL;
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float Roughness)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    
    float3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    
    float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 TangentX = normalize(cross(UpVector, N));
    float3 TangentY = cross(N, TangentX);
    
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float GeometrySchlickGGXForIntegrateBRDF(float NdotV, float Roughness)
{
    float a = Roughness;
    float k = (a * a) / 2.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmithForIntegrateBRDF(float Roughness, float NoV, float NoL)
{
    float ggx2 = GeometrySchlickGGXForIntegrateBRDF(NoV, Roughness);
    float ggx1 = GeometrySchlickGGXForIntegrateBRDF(NoL, Roughness);

    return ggx1 * ggx2;
}

float2 IntegrateBRDF(float NoV, float Roughness)
{
    float3 V;
    V.x = sqrt(1.0f - NoV * NoV); // sin
    V.y = 0;
    V.z = NoV; // cos
    float A = 0;
    float B = 0;
    const uint NumSamples = 1024;
    float3 N = float3(0.0f, 0.0f, 1.0f);
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, N, Roughness);
        float3 L = 2 * dot(V, H) * H - V;
        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));
        if (NoL > 0)
        {
            float G = GeometrySmithForIntegrateBRDF(Roughness, NoV, NoL);
            float G_Vis = G * VoH / (NoH * NoV);
            float Fc = pow(1 - VoH, 5);
            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return float2(A, B) / NumSamples;
}

#endif