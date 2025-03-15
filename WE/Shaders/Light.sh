#ifndef LIGHT_SH
#define LIGHT_SH

#ifndef DIR_LIGHTS_NUM 
#define DIR_LIGHTS_NUM 1
#endif

#ifndef POINT_LIGHTS_NUM
#define POINT_LIGHTS_NUM 0
#endif

#ifndef SPOT_LIGHTS_NUM
#define SPOT_LIGHTS_NUM 0
#endif

struct FDirectionalLight
{
    float3 Direction;
    uint Pad1;
    float3 Color;
    uint Pad2;
};

#endif