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
}

Texture2D SurfaceTexture : register(t0);
Texture2D MaskTexture : register(t1);
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
    
    float2 uvs = input.uv * scale + offset;
    float4 surfaceColor = SurfaceTexture.Sample(Sampler, uvs) * colorTint;
    float4 maskColor = MaskTexture.Sample(Sampler, uvs);
    float4 glow = float4(glowColor * glowIntensity, 1.0f) * maskColor.r;
    return surfaceColor + glow;
    //return float4(maskColor.aaa, 1.0f);

}