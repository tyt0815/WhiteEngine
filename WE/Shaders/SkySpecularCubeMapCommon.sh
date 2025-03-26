#ifndef SKYSPECULARCUBEMAPCOMMON_SH
#define SKYSPECULARCUBEMAPCOMMON_SH

cbuffer ConstantBuffers : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
    float4x4 gViewProj;
    uint gSkyCubeMapIndex;
    uint Pad1;
    uint Pad2;
    uint Pad3;
}

#endif