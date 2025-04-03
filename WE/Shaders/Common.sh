#ifndef COMMON_SH
#define COMMON_SH
#include "Math.sh"    

Texture2D gTexture[1024] : register(t0, space0);
TextureCube gTextureCube[1024] : register(t0, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

#define DECLARE_MATERIAL_SB(t, s)\
struct FMaterialSB\
{\
    uint AlbedoTextureIndex;\
    uint MetallicTextureIndex;\
    uint RoughneesTextureIndex;\
    uint NormalTextureIndex;\
    float4x4 MatTransform;\
};\
StructuredBuffer<FMaterialSB> gMaterialData : register(t, s);

#define DECLARE_PASS_CB(b)\
cbuffer PassCB : register(b)\
{\
    float4x4 gView;\
    float4x4 gInvView;\
    float4x4 gProj;\
    float4x4 gInvProj;\
    float4x4 gViewProj;\
    float4x4 gInvViewProj;\
    float3 gEyePosW;\
    uint gIndirectSpecularIntegralTextureIndex;\
    float2 gRenderTargetSize;\
    float2 gInvRenderTargetSize;\
    float gNearZ;\
    float gFarZ;\
    float gTotalTime;\
    float gDeltaTime;\
    \
    \
    float4 gFogColor;\
    float gFogStart;\
    float gFogRange;\
    uint gDirLightNum;\
    uint PassCBPad1;\
};\

#define DECLARE_MESH_CB(b)\
cbuffer MeshCB : register(b)\
{\
    float4x4 gWorld;\
    float4x4 gInvTransposeWorld;\
};

#define DECLARE_SUBMESH_CB(b)\
cbuffer SubmeshCB : register(b)\
{\
    uint gMaterialIndex;\
    uint gSkyIrradianceCubeMapIndex;\
    uint gPrefilteredSkyCubeMapIndex;\
    uint gPadOfSubmeshCB1;\
};\

void DrawSphere(in float3 InPosL, in float4x4 ViewProj, out float3 OutPosL, out float4 OutPosH)
{
    OutPosL = InPosL;
    OutPosH = mul(float4(InPosL, 1.0f), ViewProj);
}

void DrawRect(in float2 InPosition, in float2 InTexC, out float4 OutPosition, out float2 OutTexC)
{
    OutPosition = float4(InPosition, 0.0f, 1.0f);
    OutTexC = InTexC;
}

#endif