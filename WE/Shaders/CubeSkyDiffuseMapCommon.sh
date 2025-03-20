#ifndef CUBESKYDIFFUSEMAPCOMMON_SH
#define CUBESKYDIFFUSEMAPCOMMON_SH

#ifndef PI
#define PI 3.14159265359
#endif

TextureCube gTextureCube : register(t0);
Texture2D gTexture : register(t1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

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
    float3 PosL : POSITION;
    float3 NormalW : NORMAL;
};
#endif