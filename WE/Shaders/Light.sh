#ifndef LIGHT_SH
#define LIGHT_SH

#include "Math.sh"

#ifndef DIR_LIGHTS_NUM 
#define DIR_LIGHTS_NUM 1
#endif

#ifndef POINT_LIGHTS_NUM
#define POINT_LIGHTS_NUM 0
#endif

#ifndef SPOT_LIGHTS_NUM
#define SPOT_LIGHTS_NUM 0
#endif

struct FDirectionalLight
{
    float3 Direction;
    uint Pad1;
    float3 Color;
    uint Pad2;
};

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

float GeometrySmith(float3 N, float3 V, float3 L, float Roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, Roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, Roughness);
	
    return ggx1 * ggx2;
}

float3 ComputeDirectionalLight(
    FDirectionalLight Light,
    float3 V,
    float3 Albedo,
    float Metallic,
    float3 N,
    float Roughness,
    float3 F0
)
{
    float3 L = normalize(-Light.Direction);
    float3 H = normalize(V + L);
    
    // ��缱 ���
    float3 Radiance = Light.Color;
        
        // ��ũ-�䷻�� brdf
    float NDF = DistributionGGX(N, H, Roughness);
    float G = GeometrySmith(N, V, L, Roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    float3 Numerator = NDF * G * F;
    float Denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.00001f;
    float3 Specular = Numerator / Denominator;
        
    float3 kS = F;
    float3 kD = (float3) 1.0f - kS;
    kD *= 1.0f - Metallic;
        
        // ��缱�� Lo �߰�
    float NDotL = max(dot(N, L), 0.0f);
    return (kD * Albedo / PI + Specular) * Radiance * NDotL;
}

// float3 Pre

#endif