#include "Common.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix View;
    matrix Projection;
};

VTP_Sky main( VertexShaderInput input )
{
    VTP_Sky output;
    
    matrix viewNoTranslation = View;
    viewNoTranslation._14 = 0.0f;
    viewNoTranslation._24 = 0.0f;
    viewNoTranslation._34 = 0.0f;
    
    matrix vp = mul(Projection, viewNoTranslation);
    output.position = mul(vp, float4(input.localPosition, 1.0f));
    output.position.z = output.position.w;
    output.sampleDir = input.localPosition;
    return output;
}