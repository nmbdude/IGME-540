#ifndef __COMMON_SHADER_INCLUDES__
#define __COMMON_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_LIGHTS 128

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
};

struct VTP_Normal
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
};

struct VertexShaderInput
{
    float3 localPosition : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
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

struct VTP_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float SimpleFresnel(float3 n, float3 v, float f0)
{
    float NdotV = saturate(dot(n, v));
    return f0 + (1 - f0) * pow(1 - NdotV, 5);
}

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
    specular *= any(diffuse);
    return diffuse + specular;
}

float3 CalculatePointLight(Light light, float3 normal, float3 worldPos, float3 surfaceColor, float3 cameraPos, float specScale)
{
    float3 direction = normalize(worldPos - light.Position);
    float3 specular = SpecularTerm(light, normal, worldPos, cameraPos, specScale, direction) * surfaceColor;
    float3 diffuse = DiffuseColor(light, normal, direction) * surfaceColor;
    specular *= any(diffuse);
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

float4 CalculateLights(VertexToPixel input, Light lights[MAX_LIGHTS], int lightCount, Texture2D surfaceTexture, SamplerState Sampler, 
    float4 colorTint, float3 ambientColor, float2 uvScale, float2 uvOffset, float3 cameraPosition)
{
    float4 finalColor = float4(0, 0, 0, 0);
    
    input.normal = normalize(input.normal);
    float2 uvs = input.uv * uvScale + uvOffset;
    
    float4 surfaceColor = surfaceTexture.Sample(Sampler, uvs) * colorTint;
    float3 ambient = ambientColor * surfaceColor.rgb;
    float specScale = 0.5f;
    
    for (int i = 0; i < lightCount; i++)
    {
        Light light = lights[i];
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                finalColor.rgb += CalculateDirectionalLight(light, input.normal, input.worldPosition, cameraPosition, surfaceColor.rgb, specScale);
                break;
            case LIGHT_TYPE_POINT:
                finalColor.rgb += CalculatePointLight(light, input.normal, input.worldPosition, surfaceColor.rgb, cameraPosition, specScale);
                break;
            case LIGHT_TYPE_SPOT:
                finalColor.rgb += CalculateSpotLight(light, input.normal, input.worldPosition, surfaceColor.rgb, cameraPosition, specScale);
                break;
            default:
                break;
        }
    }
    
    finalColor += float4(ambient, 0);
    finalColor.a = 1;
    
    return finalColor;
}

float4 CalculateLights(VTP_Normal input, Light lights[MAX_LIGHTS], int lightCount, Texture2D surfaceTexture, SamplerState Sampler, 
    float4 colorTint, float3 ambientColor, float2 uvScale, float2 uvOffset, float3 cameraPosition)
{
    VertexToPixel vtpInput;
    vtpInput.normal = input.normal;
    vtpInput.uv = input.uv;
    vtpInput.worldPosition = input.worldPosition;
    vtpInput.screenPosition = input.screenPosition;
    return CalculateLights(vtpInput, lights, lightCount, surfaceTexture, Sampler, colorTint, ambientColor, uvScale, uvOffset, cameraPosition);
}

float3 Normals(Texture2D normalTexture, sampler Sampler, VTP_Normal input, float2 uvScale, float2 uvOffset)
{
    float3 normalSample = normalTexture.Sample(Sampler, input.uv * uvScale + uvOffset).rgb;
    float3 unpackedNormal = normalize(normalSample * 2.0f - 1.0f);
    
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 finalNormal = mul(unpackedNormal, TBN);
    return finalNormal;
}

#endif