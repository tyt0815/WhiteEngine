#include "GaussianBlurCommon.hlsli"

[numthreads(N, 1, 1)]
void MainCS(int3 GroupThreadID : SV_GroupThreadID, int3 DispatchThreadID : SV_DispatchThreadID)
{
    float Weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    
    
    if (GroupThreadID.x < gBlurRadius)
    {
        int x = max(DispatchThreadID.x - gBlurRadius, 0);
        gCache[GroupThreadID.x] = gInput[int2(x, DispatchThreadID.y)];
    }
    else if(GroupThreadID.x >= N - gBlurRadius)
    {
        int x = min(DispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
        gCache[GroupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, DispatchThreadID.y)];
    }
    
    gCache[GroupThreadID.x + gBlurRadius] = gInput[min(DispatchThreadID.xy, gInput.Length.xy - 1)];

    GroupMemoryBarrierWithGroupSync();
    
    float4 BlurColor = float4(0, 0, 0, 0);
    
    for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
    {
        int k = GroupThreadID.x + gBlurRadius + i;
        BlurColor += Weights[i + gBlurRadius] * gCache[k];
    }
    
    gOutput[DispatchThreadID.xy] = BlurColor;
}