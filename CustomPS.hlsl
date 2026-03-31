#include "Common.hlsli"

cbuffer ExternalData : register(b0)
{
    float4 color;
    float2 uv;
    float2 padding;
    float3 normal;
    float time;
}

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
    float tileCount = 4.0;
    float x = cos(input.uv.x * 6.2832 * tileCount) + time * 3;
    float y = sin(input.uv.y * 6.2832 * tileCount) + time * 2;
    return float4(cos(x) + 0.1f, sin(y) + 0.1f, sin(y * 0.25f) + 0.2f, cos(x * 0.5f) + 0.25f);
}