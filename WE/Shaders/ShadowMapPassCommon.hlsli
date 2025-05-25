#ifndef SHADOWMAPPASSCOMMON_HLSLI
#define SHADOWMAPPASSCOMMON_HLSLI
#include "Common.hlsli"
#include "Light.hlsli"

DECLARE_MATERIAL_SB(t0, space2);
DECLARE_LIGHT_SB(t0, space3);
DECLARE_MESH_CB(b1);
DECLARE_SUBMESH_CB(b2);

cbuffer ShadowMapCB : register(b3)
{
    uint gDirLightIndex;
    uint gPad1;
    uint gPad2;
    uint gPad3;
}

#endif