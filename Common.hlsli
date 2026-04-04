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

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float3 DiffuseColor(Light light, float3 normal, float3 direction)
{
    return saturate(dot(normal, -direction)) * light.Color * light.intensity;
}

float3 SpecularTerm(Light light, float3 normal, float3 worldPos, float3 cameraPos, float specScale, float3 direction)
{
    float3 directionToCamera = normalize(cameraPos - worldPos);
    float3 reflectDir = reflect(direction, normal);
    float RdotV = saturate(dot(reflectDir, directionToCamera));
    return pow(RdotV, 256.0f) * specScale * light.intensity * light.Color;
}

float3 CalculateDirectionalLight(Light light, float3 normal, float3 worldPos, float3 cameraPos, float3 surfaceColor, float specScale)
{
    float3 diffuse = DiffuseColor(light, normal, normalize(light.Direction)) * surfaceColor;
    float3 specular = SpecularTerm(light, normal, worldPos, cameraPos, specScale, normalize(light.Direction)) * surfaceColor;
    return diffuse + specular;
}

float3 CalculatePointLight(Light light, float3 normal, float3 worldPos, float3 surfaceColor, float3 cameraPos, float specScale)
{
    float3 direction = normalize(worldPos - light.Position);
    float specular = SpecularTerm(light, normal, worldPos, cameraPos, specScale, direction) * surfaceColor;
    float3 diffuse = DiffuseColor(light, normal, direction) * surfaceColor;
    return (diffuse + specular) * Attenuate(light, worldPos);
}

float3 CalculateSpotLight(Light light, float3 normal, float3 worldPos, float3 surfaceColor, float3 cameraPos, float specScale)
{
    float3 toPixel = normalize(worldPos - light.Position);
    float pixelAngle = saturate(dot(toPixel, -normalize(light.Direction)));
    
    float cosOuter = cos(radians(light.SpotOuterAngle));
    float cosInner = cos(radians(light.SpotInnerAngle));
    float falloffRange = cosOuter - cosInner;
    
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    float3 finalColor = CalculatePointLight(light, normal, worldPos, surfaceColor, cameraPos, specScale) * spotTerm;
    return finalColor;
}

#endif