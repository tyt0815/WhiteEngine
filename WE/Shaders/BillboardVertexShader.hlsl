#include "BillboardCommon.hlsli"

FVSOutput MainVS(FVSInput VIn)
{
    FVSOutput VOut;
    
    VOut.CenterW = mul(float4(VIn.PosL, 1.0f), gWorld);
    VOut.SizeW = VIn.SizeW;
    
    return VOut;
}