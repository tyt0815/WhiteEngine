#ifndef SKYATMOSPHERECOMMON_HLSLI
#define SKYATMOSPHERECOMMON_HLSLI
#include "Common.hlsli"

#ifndef TRANSMITTANCE_TEXTURE_WIDTH
#define TRANSMITTANCE_TEXTURE_WIDTH 128
#endif
#ifndef TRANSMITTANCE_TEXTURE_HEIGHT
#define TRANSMITTANCE_TEXTURE_HEIGHT 128
#endif

struct FDensityProfileLayer
{
    float Width;
    float ExpTerm;
    float ExpScale;
    float LinearTerm;
    float ConstantTerm;
};

struct FDensityProfile
{
    FDensityProfileLayer Layer0;
    FDensityProfileLayer Layer1;
};

cbuffer SkyAtmosphereCommonConstantBuffers : register(b0)
{
    // The solar irradiance at the top of the atmosphere.
    float3 gSolarIrradiance;
    // The sun's angular radius. Warning: the implementation uses approximations
    // that are valid only if this angle is smaller than 0.1 radians.
    float gSunAngularRadius;
    
    // The distance between the planet center and the bottom of the atmosphere.
    float gBottomRadius;
    // The distance between the planet center and the top of the atmosphere.
    float gTopRadius;
    // The density profile of air molecules, i.e. a function from altitude to
    // dimensionless values between 0 (null density) and 1 (maximum density).
    FDensityProfile gRayleighDensity;
    
    // The scattering coefficient of air molecules at the altitude where their
    // density is maximum (usually the bottom of the atmosphere), as a function of
    // wavelength. The scattering coefficient at altitude h is equal to
    // 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
    float3 gRayleighScattering;
    // The density profile of aerosols, i.e. a function from altitude to
    // dimensionless values between 0 (null density) and 1 (maximum density).
    FDensityProfile gMieDensity;
    // The scattering coefficient of aerosols at the altitude where their density
    // is maximum (usually the bottom of the atmosphere), as a function of
    // wavelength. The scattering coefficient at altitude h is equal to
    // 'mie_scattering' times 'mie_density' at this altitude.
    float3 gMieScattering;
    
    // The extinction coefficient of aerosols at the altitude where their density
    // is maximum (usually the bottom of the atmosphere), as a function of
    // wavelength. The extinction coefficient at altitude h is equal to
    // 'mie_extinction' times 'mie_density' at this altitude.
    float3 gMieExtinction;
    // The asymetry parameter for the Cornette-Shanks phase function for the
    // aerosols.
    float gMiePhaseFunction_g;
    
    // The density profile of air molecules that absorb light (e.g. ozone), i.e.
    // a function from altitude to dimensionless values between 0 (null density)
    // and 1 (maximum density).
    FDensityProfile gAbsorptionDensity;
    // The extinction coefficient of molecules that absorb light (e.g. ozone) at
    // the altitude where their density is maximum, as a function of wavelength.
    // The extinction coefficient at altitude h is equal to
    // 'absorption_extinction' times 'absorption_density' at this altitude.
    float3 gAbsorptionExtinction;
    // The average albedo of the ground;
    float3 gGroundAlbedo;
    
    // The cosine of the maximum Sun zenith angle for which atmospheric scattering
    // must be precomputed (for maximum precision, use the smallest Sun zenith
    // angle yielding negligible sky light radiance values. For instance, for the
    // Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
    float g_mu_s_Min;
    uint SkyAtmosphereCommonCBPad1;
    uint SkyAtmosphereCommonCBPad2;
    uint SkyAtmosphereCommonCBPad3;
}

float DistanceToTopAtmosphereBoundary(float r, float mu)
{
    float Discriminant = max(r * r * (mu * mu - 1.0f) + gTopRadius * gTopRadius, 0.0f);
    return max(-r * mu + sqrt(Discriminant), 0.0f);
}

float DistanceToBottomAtmosphereBoundary(float r, float mu)
{
    float Discriminant = max(r * r * (mu * mu - 1.0f) + gBottomRadius * gBottomRadius, 0.0f);
    return max(-r * mu - sqrt(Discriminant), 0.0f);
}

bool RayIntersectsGround(float r, float mu)
{
    return mu < 0.0f && r * r * (mu * mu - 1.0f) + gBottomRadius * gBottomRadius >= 0.0f;
}

float GetLayerDensity(in FDensityProfileLayer Layer, float Altitude)
{
    float Density = Layer.ExpTerm * exp(Layer.ExpScale * Altitude) + Layer.LinearTerm * Altitude + Layer.ConstantTerm;
    return saturate(Density);
}

float GetProfileDensity(in FDensityProfile Profile, float Altitude)
{
    return Altitude < Profile.Layer0.Width ?
    GetLayerDensity(Profile.Layer0, Altitude) : GetLayerDensity(Profile.Layer1, Altitude);
}

float ComputeOpticalLengthToTopAtmosphereBoundary(in FDensityProfile Profile, float r, float mu)
{
    const int SAMPLE_COUNT = 500;
    float dx = DistanceToTopAtmosphereBoundary(r, mu) / float(SAMPLE_COUNT);
    float Result = 0.0f;
    for (int i = 0; i <= SAMPLE_COUNT; ++i)
    {
        float d_i = float(i) * dx;
        float r_i = sqrt(d_i * d_i + 2.0f * r * mu * d_i + r * r);
        float y_i = GetProfileDensity(Profile, r_i - gBottomRadius);
        float Weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f;
        Result += y_i * Weight_i * dx;
    }
    return Result;
}

float3 ComputeTransmittanceToTopAtmosphereBoundary(float r, float mu)
{
    return exp(
        -(
            gRayleighScattering * ComputeOpticalLengthToTopAtmosphereBoundary(gRayleighDensity, r, mu) +
            gMieExtinction * ComputeOpticalLengthToTopAtmosphereBoundary(gMieDensity, r, mu) +
            gAbsorptionExtinction * ComputeOpticalLengthToTopAtmosphereBoundary(gAbsorptionDensity, r, mu)
        )
    );
}

float GetTextureCoordFromUnitRange(float x, int TextureSize)
{
    return 0.5f / float(TextureSize) + x * (1.0f - 1.0f / float(TextureSize));
}

float GetUnitRangeFromTextureCoord(float u, int TextureSize)
{
    return (u - 0.5f / float(TextureSize)) / (1.0f - 1.0f / float(TextureSize));
}

float2 GetTransmittanceTextureUvFromRMu(float r, float mu)
{
    float H = sqrt(gTopRadius * gTopRadius - gBottomRadius * gBottomRadius);
    float rho = sqrt(max(r * r - gBottomRadius * gBottomRadius, 0.0f));
    float d = DistanceToTopAtmosphereBoundary(r, mu);
    float d_min = gTopRadius - r;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;
    return float2(
        GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
        GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT)
    );
}

void GetRMuFromTransmittanceTextureUv(in float2 TexC, out float r, out float mu)
{
    float x_mu = GetUnitRangeFromTextureCoord(TexC.x, TRANSMITTANCE_TEXTURE_WIDTH);
    float x_r = GetUnitRangeFromTextureCoord(TexC.y, TRANSMITTANCE_TEXTURE_HEIGHT);
    float H = sqrt(gTopRadius * gTopRadius - gBottomRadius * gBottomRadius);
    float rho = H * x_r;
    r = sqrt(rho * rho + gBottomRadius * gBottomRadius);
    float d_min = gTopRadius - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    mu = d == 0.0f ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * r * d);
    mu = clamp(mu, -1.0f, 1.0f);
}

float3 GetTransmittanceToTopAtmosphereBoundary(in Texture2D TransmittanceTexture, float r, float mu)
{
    float2 uv = GetTransmittanceTextureUvFromRMu(r, mu);
    return TransmittanceTexture.Sample(gsamLinearClamp, uv).rgb;
}

float3 GetTransmittance(in Texture2D TransmittanceTexture, float r, float mu, float d, bool bRMuIntersectsGround)
{
    float r_d = clamp(sqrt(d * d + 2.0f * r * mu * d + r * r), gBottomRadius, gTopRadius);
    float mu_d = clamp((r * mu + d), -1.0f, 1.0f);
    if(bRMuIntersectsGround)
    {
        return min(
            GetTransmittanceToTopAtmosphereBoundary(TransmittanceTexture, r_d, -mu_d) /
            GetTransmittanceToTopAtmosphereBoundary(TransmittanceTexture, r, -mu),
            float3(1.0f, 1.0f, 1.0f)
        );
    }
    else
    {
        return min(
            GetTransmittanceToTopAtmosphereBoundary(TransmittanceTexture, r, mu)/
            GetTransmittanceToTopAtmosphereBoundary(TransmittanceTexture, r_d, mu_d),
            float3(1.0f, 1.0f, 1.0f)
        );
    }

}

float3 GetTransmittanceToSun(in Texture2D TransmittanceTexture, float r, float mu_s)
{
    float sin_theta_h = gBottomRadius / r;
    float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0f));
    return GetTransmittanceToTopAtmosphereBoundary(TransmittanceTexture, r, mu_s) *
        smoothstep(-sin_theta_h * gSunAngularRadius, sin_theta_h * gSunAngularRadius, mu_s - cos_theta_h);
}

#endif