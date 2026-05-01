#include "Common.hlsli"

cbuffer fogData : register(b0)
{
    float3 fogColor;
    bool enableFog;
    float fogDensity;
    float3 cameraPosition;
}

TextureCube SkyTexture : register(t0);
SamplerState SkySampler : register(s0);

float4 main(VTP_Sky input) : SV_TARGET
{
    float4 finalColor = SkyTexture.Sample(SkySampler, input.sampleDir);
    if (enableFog)
    {
        float distance = length(cameraPosition - input.worldPos);
        float fog = 1 - exp(-distance * fogDensity);
        float3 finalResult = lerp(finalColor.rgb, fogColor, fog);
        return float4(pow(finalResult, 1.0 / 2.2f), 1);
    }
    return finalColor;
}