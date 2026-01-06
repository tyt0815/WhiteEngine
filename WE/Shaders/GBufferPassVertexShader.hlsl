#include "GBufferPassCommon.hlsli"

void MainVS(
    in FVertexInput VIn,
    out float4 ClipPosition : SV_Position,
    out float3 WorldNormal : NORMAL,
    out float2 TexC : TEXCOORD,
    out float3 WorldTangent : TANGENT
)
{
    FMaterialSB MaterialInfo = gMaterialData[gMaterialIndex];
    
#ifdef SKINNED
    float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weights[0] = VIn.BoneWeights.x;
    weights[1] = VIn.BoneWeights.y;
    weights[2] = VIn.BoneWeights.z;
    weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    for(int i = 0; i < 4; ++i)
    {
        // Assume no nonuniform scaling when transforming normals, so 
        // that we do not have to use the inverse-transpose.

        posL += weights[i] * mul(float4(VIn.LocalPosition, 1.0f), gBoneTransforms[VIn.BoneIndices[i]]).xyz;
        normalL += weights[i] * mul(VIn.LocalNormal, (float3x3)gBoneTransforms[VIn.BoneIndices[i]]);
        tangentL += weights[i] * mul(VIn.TangentU.xyz, (float3x3)gBoneTransforms[VIn.BoneIndices[i]]);
    }

    VIn.LocalPosition = posL;
    VIn.LocalNormal = normalL;
    VIn.TangentU = tangentL;
#endif
    
    float4 PosW = mul(float4(VIn.LocalPosition, 1.0f), gWorld);
    
    ClipPosition = mul(PosW, gViewProj);
    ClipPosition = mul(float4(VIn.LocalPosition, 1.0f), gViewProj);
#ifdef SKINNED
    ClipPosition = mul(float4(VIn.LocalPosition, 1.0f), gViewProj);
#endif
    WorldNormal = mul(VIn.LocalNormal, (float3x3) gInvTransposeWorld);
    TexC = mul(float4(VIn.TexC, 0.0f, 1.0f), MaterialInfo.MatTransform).xy;
    WorldTangent = mul(VIn.TangentU.xyz, (float3x3) gInvTransposeWorld);
}