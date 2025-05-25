#ifndef GAUSSIANBLURCOMMON_HLSLI
#define GAUSSIANBLURCOMMON_HLSLI

cbuffer GaussianBlurConstantBuffers : register(b0)
{
    int gBlurRadius;
    
    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
}

#define BLUR_RADIUS_MAX 5
#define N 256
#define CACHE_SIZE (N + 2 * BLUR_RADIUS_MAX)

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);
groupshared float4 gCache[CACHE_SIZE];

#endif