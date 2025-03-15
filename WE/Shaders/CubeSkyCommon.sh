#ifndef CUBESKYCOMMON_SH
#define CUBESKYCOMMON_SH
#include "Common.sh"

struct FVertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct FVertexOut
{
    float4 PosH : SV_Position;
    float3 PosL : POSITIONT;
};

#endif