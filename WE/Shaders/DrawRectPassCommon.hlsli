#ifndef DRAWRECTPASSCOMMON_HLSLI
#define DRAWRECTPASSCOMMON_HLSLI
#include "Common.hlsli"

cbuffer CB : register(b0)
{
	uint gTextureIndex;
	uint Pad1;
	uint Pad2;
	uint Pad3;
}

#endif // !DRAWRECTPASSCOMMON_HLSLI

