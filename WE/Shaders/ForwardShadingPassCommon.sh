#ifndef FORWARDSHADINGPASSCOMMON_SH
#define FORWARDSHADINGPASSCOMMON_SH
#include "Common.sh"
#include "Light.sh"

StructuredBuffer<FDirectionalLight> gDirectionalLights : register(t0, space3);

DECLARE_MATERIAL_SB(t0, space2);
DECLARE_PASS_CB(b0);
DECLARE_MESH_CB(b1);
DECLARE_SUBMESH_CB(b2);

struct FVSInput
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct FVSOutput
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
};

float3 ComputeLight(
    in FMaterial Material,
    in float3 N,
    in float3 V
)
{
    float3 F0 = (float3) 0.04;
    F0 = lerp(F0, Material.Albedo, Material.Metallic);
    
    float3 Lo = (float3) 0.0f;
    
    // DirectionalLight 
    for (uint i = 0; i < gDirLightNum; ++i)
    {
        Lo += ComputeDirectionalLight(gDirectionalLights[i], Material, V, N, F0);
    }
    
    
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, Material.Roughness);
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - Material.Metallic;
    
    // IBL Diffuse
    float3 Irradiance = gTextureCube[gSkyIrradianceCubeMapIndex].Sample(gsamLinearWrap, N).rgb;
    float3 Diffuse = Irradiance * Material.Albedo;
    // Diffuse = pow(Diffuse, 3.0f);
    // return float4(Diffuse * Diffuse, 1.0f);
    
    // IBL Specular
    float3 R = reflect(-V, N);
    const float MAX_REFLECTION_LOD = 5.0;
    float3 prefilteredColor = gTextureCube[gPrefilteredSkyCubeMapIndex].SampleLevel(gsamLinearWrap, R, Material.Roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = gTexture[gIndirectSpecularIntegralTextureIndex].Sample(gsamLinearWrap, float2(max(dot(N, V), 0.0), Material.Roughness)).rg;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    // TODO: AO 적용. float3(1.0f)를 AO로 변경
    float3 Ambient = (kD * Diffuse + specular) * (float3) 1.0f;
    
    // 디버깅용. IBL Diffuse까지 적용
    // Ambient = (kD * Diffuse);
    
    // 디버깅용. BRDF만 적용
    // Ambient = (float3) 0.03f * Material.Albedo;
    
    
    float3 Color = Ambient + Lo;
    // HDR tone mapping
    Color = Color / (Color + (float3) 1.0f);
    // gamma correct
    Color = pow(Color, (float3) (1.0f / 2.2f));
    return Color;
}

#endif