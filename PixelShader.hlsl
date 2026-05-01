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
    float3 fogColor;
    bool enableFog;
    float fogDensity;
}

Texture2D SurfaceTexture : register(t0);
Texture2D SpecularMap : register(t1);
Texture2D ShadowMap : register(t4);
TextureCube SkyTexture : register(t100);
SamplerState Sampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
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
    
    finalColor += CalculateLights(input, lights, lightCount, SurfaceTexture, Sampler,
    colorTint, ambientColor, scale, offset, cameraPosition, shadowAmount);
    
    float3 viewVector = normalize(cameraPosition - input.worldPosition);
    float3 reflectionVector = reflect(-viewVector, input.normal);
    float3 reflectionColor = SkyTexture.Sample(Sampler, reflectionVector).rgb;
    float3 result = lerp(finalColor.rgb, reflectionColor, SimpleFresnel(input.normal, viewVector, 0.04f));
    if(enableFog)
    {
        float distance = length(cameraPosition - input.worldPosition);
        float fog = 1 - exp(-distance * fogDensity);
        float3 finalResult = lerp(result, fogColor, fog);
        return float4(pow(finalResult, 1.0 / 2.2f), 1);
    }
    return float4(pow(result, 1.0 / 2.2f), 1);
}