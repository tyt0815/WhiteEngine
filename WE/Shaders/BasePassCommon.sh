#ifndef BASEPASSCOMMON_SH
#define BASEPASSCOMMON_SH

cbuffer ConstantBufferPerPass : register(b0)
{
    float4x4 View;
    float4x4 Project;
};

struct FBasePassVSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct FBasePassVSOutput
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

#endif