#include "Common.hlsli"

DECLARE_PASS_CB(b0)

struct FVertexIn
{
	float3 Pos : POSITION;
	float4 Color : COLOR;
};

struct FVertexOut
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};

FVertexOut MainVS(in FVertexIn VIn)
{
	FVertexOut VOut;

	VOut.Pos = mul(float4(VIn.Pos, 1.0f), gViewProj);
	VOut.Color = VIn.Color;

	return VOut;
}

float4 MainPS(in FVertexOut PIn) : SV_TARGET
{
	return PIn.Color;
}