#ifndef __COMMON_SHADER_INCLUDES__
#define __COMMON_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};

struct VertexShaderInput
{
    float3 localPosition : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

float3 NormalizedDirection(Light light, float3 worldPosition)
{
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        return -light.Direction;
    }
    else
    {
        return normalize(light.Position - worldPosition);
    }
}

float3 DiffuseColor(Light light, float3 normal)
{
    return saturate(dot(normal, -light.Direction)) * light.Color * light.intensity;
}

float3 SpecularTerm(Light light, float3 normal, float3 worldPos, float3 cameraPos, float specScale)
{
    float3 directionToCamera = normalize(cameraPos - worldPos);
    float3 reflectDir = reflect(light.Direction, normal);
    float RdotV = saturate(dot(reflectDir, directionToCamera));
    return pow(RdotV, 64.0f) * specScale * light.intensity * light.Color;
}

float3 CalculateDirectionalLight(Light light, float3 normal, float3 worldPos, float3 cameraPos, float3 surfaceColor, float specScale)
{
    float3 diffuse = DiffuseColor(light, normal) * surfaceColor;
    float3 specular = SpecularTerm(light, normal, worldPos, cameraPos, specScale) * surfaceColor;
    return diffuse + specular;
}

#endif