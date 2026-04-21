#include "Common.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix worldMatrix;
    matrix view;
    matrix projection;
    matrix worldInverseTranspose;
}

VTP_Normal main(VertexShaderInput input)
{
    VTP_Normal output;
    matrix wvp = mul(projection, mul(view, worldMatrix));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.worldPosition = mul(worldMatrix, float4(input.localPosition, 1.0f)).xyz;
    output.uv = input.uv;
    output.normal = mul((float3x3) worldInverseTranspose, input.normal);
    output.tangent = mul((float3x3) worldMatrix, input.tangent);
    return output;
}