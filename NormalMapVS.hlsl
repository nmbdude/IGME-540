#include "Common.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix worldMatrix;
    matrix view;
    matrix projection;
    matrix worldInverseTranspose;
    matrix lightView;
    matrix lightProjection;
}

VTP_Normal main(VertexShaderInput input)
{
    VTP_Normal output;
    matrix wvp = mul(projection, mul(view, worldMatrix));
    matrix shadowWVP = mul(lightProjection, mul(lightView, worldMatrix));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
    output.worldPosition = mul(worldMatrix, float4(input.localPosition, 1.0f)).xyz;
    output.uv = input.uv;
    output.normal = mul((float3x3) worldInverseTranspose, input.normal);
    output.tangent = mul((float3x3) worldMatrix, input.tangent);
    output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));
    return output;
}