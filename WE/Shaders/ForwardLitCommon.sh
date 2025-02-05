#ifndef BASEPASSCOMMON_SH
#define BASEPASSCOMMON_SH

#include "Common.sh"

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