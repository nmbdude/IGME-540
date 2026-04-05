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
SamplerState Sampler : register(s0);

float4 main(VTP_Normal input) : SV_TARGET
{
    float4 finalColor = float4(0, 0, 0, 0);

    input.normal = normalize(Normals(NormalMap, Sampler, input, scale, offset));
    finalColor += CalculateLights(input, lights, lightCount, SurfaceTexture, Sampler,
    colorTint, ambientColor, scale, offset, cameraPosition);
    
    return finalColor;
}