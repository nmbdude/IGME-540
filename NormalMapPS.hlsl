#include "Common.hlsli"


cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uv;
    float2 padding;
    float3 normal;
    float time;
    float2 scale;
    float2 offset;
    float3 glowColor;
    float glowIntensity;
    float3 cameraPosition;
    float padding2;
    float3 ambientColor;
    float padding3;
    Light lights[MAX_LIGHTS];
    int lightCount;
}

Texture2D SurfaceTexture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D ShadowMap : register(t4);
TextureCube SkyTexture : register(t100);
SamplerState Sampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// Needed to add this to remove a weird error/warning from VS saying the Normals function didn't take 5 arguments
float3 Normals(Texture2D normalTexture, SamplerState Sampler, VTP_Normal input, float2 uvScale, float2 uvOffset);


float4 main(VTP_Normal input) : SV_TARGET
{
    float4 finalColor = float4(0, 0, 0, 0);

    input.shadowMapPos /= input.shadowMapPos.w;
    
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y;
    
    float distanceToLight = input.shadowMapPos.z;

    float shadowAmount = ShadowMap.SampleCmpLevelZero(
        ShadowSampler,
        shadowUV,
        distanceToLight).r;
    
    input.normal = normalize(Normals(NormalMap, Sampler, input, scale, offset));
    finalColor += CalculateLights(input, lights, lightCount, SurfaceTexture, Sampler,
    colorTint, ambientColor, scale, offset, cameraPosition, shadowAmount);
    
    float3 viewVector = normalize(cameraPosition - input.worldPosition);
    float3 reflectionVector = reflect(-viewVector, input.normal);
    float3 reflectionColor = SkyTexture.Sample(Sampler, reflectionVector).rgb;
    float3 result = lerp(finalColor.rgb, reflectionColor, SimpleFresnel(input.normal, viewVector, 0.04f));
    return float4(pow(result, 1.0/2.2f), 1);
}