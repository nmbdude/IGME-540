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
    Light lights[5];
}

Texture2D SurfaceTexture : register(t0);
Texture2D SpecularMap : register(t1);
SamplerState Sampler : register(s0);


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
    
    input.normal = normalize(input.normal);
    float2 uvs = input.uv * scale + offset;
    
    float4 surfaceColor = SurfaceTexture.Sample(Sampler, uvs) * colorTint;
    float3 ambient = ambientColor * surfaceColor.rgb;
    float specScale = 0.5f;
    
    for (int i = 0; i < 4; i++)
    {
        finalColor += float4(CalculateDirectionalLight(lights[i], input.normal, input.worldPosition, cameraPosition, surfaceColor.rgb, specScale), 1);
    }
    finalColor += float4(CalculatePointLight(lights[4], input.normal, input.worldPosition, surfaceColor.rgb, cameraPosition, specScale), 1);
    return float4(CalculateSpotLight(lights[3], input.normal, input.worldPosition, surfaceColor.rgb, cameraPosition, specScale), 1);
    finalColor += float4(CalculateSpotLight(lights[3], input.normal, input.worldPosition, surfaceColor.rgb, cameraPosition, specScale), 1);
    
    finalColor += float4(ambient, 0);
    finalColor.a = 1;
    
    return finalColor;
}