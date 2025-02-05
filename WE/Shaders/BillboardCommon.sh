#ifndef BILLBOARDCOMMON_SH
#define BILLBOARDCOMMON_SH

#include "Common.sh"

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