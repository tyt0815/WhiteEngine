#ifndef MATERIAL_SH
#define MATERIAL_SH

struct FMaterial
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};

#endif