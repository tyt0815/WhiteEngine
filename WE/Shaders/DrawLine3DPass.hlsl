#include "DrawShapeCommon.hlsli"

struct FVertexInput
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct FVertexOutput
{
	float4 Position : SV_POSITION;
	float4 Color : Color;
};

FVertexOutput MainVS(FVertexInput VIn)
{
	FVertexOutput VOut;

	VOut.Position = mul(float4(VIn.Position, 1.0f), g_ViewProj);
	VOut.Color = VIn.Color;

	return VOut;
}

float4 MainPS(FVertexOutput PIn) : Sv_Target
{
	return PIn.Color;
}