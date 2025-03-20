#ifndef CUBESKYDIFFUSEMAPCOMMON_SH
#define CUBESKYDIFFUSEMAPCOMMON_SH

TextureCube gTextureCube : register(t0);
cbuffer FTempCB : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
}

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