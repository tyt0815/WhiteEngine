#ifndef COMMON_HLSLI
#define COMMON_HLSLI
#include "Math.hlsli"

#ifndef TEXTURES
#define TEXTURES
Texture2D gTexture[1024] : register(t0, space0);
TextureCube gTextureCube[1024] : register(t0, space1);
#endif

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
    uint PassCBPad1;\
    uint PassCBPad2;\
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

#define DECLARE_LIGHTINFO_CB(b)\
cbuffer LightInfoCB : register(b)\
{\
    uint gDirectionalLightNum;\
    uint gPointLightNum;\
    uint gSpotLightNum;\
    uint gLightInfoPad1;\
};

struct FVertexInput
{
    float3 LocalPosition : POSITION;
    float3 LocalNormal : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

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

float CalcNDCDepthToViewDepth(float z, float4x4 Proj)
{
    float ViewZ = Proj[3][4] / (z - Proj[2][2]);
    return ViewZ;
}

// ScreenÁÂÇ¥ -> View ÁÂÇ¥°è
float3 TransformScreenToView(in float2 TexC, float Depth, in float4x4 InvProj)
{
    float4 NDCPosition = float4(TexC * 2 - 1.0f, Depth, 1.0f);
    NDCPosition.y *= -1;
    float4 ViewPosition = mul(NDCPosition, InvProj);
    ViewPosition /= ViewPosition.w;
    return ViewPosition.xyz;
}

// View ÁÂÇ¥°è -> World ÁÂÇ¥°è
float3 TransformViewToWorld(in float3 ViewPosition, in float4x4 InvView)
{
    float4 WorldPosition = mul(float4(ViewPosition, 1.0f), InvView);
    return WorldPosition.xyz;
}

// ScreenÁÂÇ¥°è -> World ÁÂÇ¥°è
float3 TransformScreenToWorld(in float2 TexC, float Depth, in float4x4 InvProj, in float4x4 InvView)
{
    float3 ViewPosition = TransformScreenToView(TexC, Depth, InvProj);
    return TransformViewToWorld(ViewPosition, InvView);
}

// º¤ÅÍ¸¦ normalizeÇÏ°í 0 ~ 1°ªÀ¸·Î ÀÎÄÚµù
float3 EncodeVector(float3 Vector)
{
    return (normalize(Vector) + 1.0f) / 2.0f;
}

// 0 ~ 1·Î ÀÎÄÚµùµÈ º¤ÅÍ¸¦ -1 ~ 1·Î µðÄÚµù
float3 DecodeVector(float3 EncodedVector)
{
    return EncodedVector * 2 - 1.0f;
}

float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 UnitNormal, float3 TangentW)
{
    float3 NormalT = DecodeVector(NormalMapSample);

    float3 N = UnitNormal;
    float3 T = normalize(TangentW - dot(TangentW, N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    float3 WorldNormal = mul(NormalT, TBN);
    return WorldNormal;
}

#endif