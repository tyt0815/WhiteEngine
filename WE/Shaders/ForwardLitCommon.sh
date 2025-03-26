#ifndef FORWARDLITCOMMON_SH
#define FORWARDLITCOMMON_SH
#include "Common.sh"
#include "Light.sh"

struct FMaterialSB
{
    uint AlbedoTextureIndex;
    uint MetallicTextureIndex;
    uint RoughneesTextureIndex;
    uint NormalTextureIndex;
    float4x4 MatTransform;
};
StructuredBuffer<FMaterialSB> gMaterialData : register(t0, space2);


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
    
    FDirectionalLight gDirectionalLights[DIR_LIGHTS_NUM];
};

cbuffer ConstantBufferPerObject : register(b1)
{
    float4x4 gWorld;
};

cbuffer SubmeshCB : register(b2)
{
    uint gMaterialIndex;
    uint gSkyIrradianceCubeMapIndex;
    uint gSkySpecularCubeMapIndex;
    uint gPadOfSubmeshCB1;
};


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

#endif