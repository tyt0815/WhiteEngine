#ifndef COMMON_SH
#define COMMON_SH
#include "Light.sh"

struct FMaterialSB
{
    uint AlbedoTextureIndex;
    uint MetallicTextureIndex;
    uint RoughneesTextureIndex;
    uint NormalTextureIndex;
    float4x4 MatTransform;
};


TextureCube gTextureCube : register(t0);
Texture2D gTexture[1000] : register(t1);
StructuredBuffer<FMaterialSB> gMaterialData : register(t0, space1);
SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer ConstantBufferPerPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float gcbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    
    
    // Fog Info
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerObjectPad2;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    FDirectionalLight gDirectionalLights[DIR_LIGHTS_NUM];
};

cbuffer ConstantBufferPerObject : register(b1)
{
    float4x4 gWorld;
};

cbuffer SubmeshCB : register(b2)
{
    uint gMaterialIndex;
    uint gPadOfSubmeshCB1;
    uint gPadOfSubmeshCB2;
    uint gPadOfSubmeshCB3;
};

#endif