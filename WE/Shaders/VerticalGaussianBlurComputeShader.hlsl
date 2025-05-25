#include "GaussianBlurCommon.hlsli"

[numthreads(1, N, 1)]
void MainCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
    float Weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    
    
    if (GroupThreadID.y < gBlurRadius)
    {
        int y = max(DispatchThreadID.y - gBlurRadius, 0);
        gCache[GroupThreadID.y] = gInput[int2(DispatchThreadID.x, y)];
    }
    else if (GroupThreadID.y >= N - gBlurRadius)
    {
        int y = min(DispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
        gCache[GroupThreadID.y + 2 * gBlurRadius] = gInput[int2(DispatchThreadID.x, y)];
    }
    
    gCache[GroupThreadID.y + gBlurRadius] = gInput[min(DispatchThreadID.xy, gInput.Length.xy - 1)];

    GroupMemoryBarrierWithGroupSync();
    
    float4 BlurColor = float4(0, 0, 0, 0);
    
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = GroupThreadID.y + gBlurRadius + i;
        BlurColor += Weights[i + gBlurRadius] * gCache[k];
    }
    
    gOutput[DispatchThreadID.xy] = BlurColor;
}