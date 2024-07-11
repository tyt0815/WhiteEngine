#include "UnlitCommon.hlsl"

float4 PSMain(VertexOut pin) : SV_Target
{
    return pin.Color;
}
