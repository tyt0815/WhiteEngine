#ifndef BILLBOARDCOMMON_HLSLI
#define BILLBOARDCOMMON_HLSLI

#include "Common.hlsli"

struct FVSInput
{
    float3 PosL : POSITION;
    float2 SizeW : SIZE;
};

struct FVSOutput
{
    float3 CenterW : POSTION;
    float2 SizeW : SIZE;
};

struct FGSOutput
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    uint PrimID : SV_PrimitiveID;
};

#endif