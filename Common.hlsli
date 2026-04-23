#ifndef __COMMON_SHADER_INCLUDES__
#define __COMMON_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_LIGHTS 16
#define PI 3.14159265359
#define MIN_ROUGHNESS 0.0000001f

struct VertexToPixel
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float4 shadowMapPos : SHADOW_POSITION;
};

struct VTP_Normal
{
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
    float4 shadowMapPos : SHADOW_POSITION;
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
    float4 colorTint, float3 ambientColor, float2 uvScale, float2 uvOffset, float3 cameraPosition, float shadowAmount)
{
    float4 finalColor = float4(0, 0, 0, 0);
    
    input.normal = normalize(input.normal);
    float2 uvs = input.uv * uvScale + uvOffset;
    
    float4 surfaceColor = surfaceTexture.Sample(Sampler, uvs) * colorTint;
    surfaceColor.rgb = pow(surfaceColor.rgb, 2.2f);
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
        
        if (i == 0)
        {
            finalColor.rgb *= shadowAmount;
        }
    }
    
    //finalColor += float4(ambient, 0);
    finalColor.a = 1;
    
    return finalColor;
}

float4 CalculateLights(VTP_Normal input, Light lights[MAX_LIGHTS], int lightCount, Texture2D surfaceTexture, SamplerState Sampler, 
    float4 colorTint, float3 ambientColor, float2 uvScale, float2 uvOffset, float3 cameraPosition, float shadowAmount)
{
    VertexToPixel vtpInput;
    vtpInput.normal = input.normal;
    vtpInput.uv = input.uv;
    vtpInput.worldPosition = input.worldPosition;
    vtpInput.screenPosition = input.screenPosition;
    return CalculateLights(vtpInput, lights, lightCount, surfaceTexture, Sampler, colorTint, ambientColor, uvScale, uvOffset, cameraPosition, shadowAmount);
}

float3 Normals(Texture2D normalTexture, SamplerState Sampler, VTP_Normal input, float2 uvScale, float2 uvOffset)
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



// PBR ----------------------------------

float D_GGX(float3 n, float3 h, float roughness)
{
    float NdotH = saturate(dot(n, h));
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS);

    float denomToSquare = (NdotH * NdotH) * (a2 - 1) + 1;
    return a2 / (PI * denomToSquare * denomToSquare);
}

float G_SchlickGGX(float3 n, float3 v, float roughness)
{
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));
    return 1 / (NdotV * (1 - k) + k);
}

float3 F_Schlick(float3 v, float3 h, float3 f0)
{
    float VdotH = saturate(dot(v, h));
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}

float3 PBRDiffuse(float3 normal, float3 direction)
{
    return saturate(dot(normal, -direction));
}

float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 O_F)
{
    float3 h = normalize(v + l);

    float D = D_GGX(n, h, roughness);
    O_F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
    
    return (D * O_F * G) / 4;
}

float3 PBRDirectionalLight(Light light, float3 normal, float3 albedoColor, float3 toCamera, float3 specularColor, float roughness, float metalness)
{
    float3 F;
    float3 diffuse = PBRDiffuse(normal, normalize(light.Direction));
    float3 specular = MicrofacetBRDF(normal, normalize(-light.Direction),
        toCamera, roughness, specularColor, F);
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
    float3 total = (balancedDiff * albedoColor + specular) * light.intensity * light.Color;
    
    return total;
}

float3 PBRPointLight(Light light, float3 normal, float3 worldPos, float3 albedoColor, float3 toCamera, float3 specularColor, float roughness, float metalness)
{
    float3 F;
    float3 direction = normalize(light.Position - worldPos);
    float3 diffuse = PBRDiffuse(normal, direction);
    float3 specular = MicrofacetBRDF(normal, direction,
        toCamera, roughness, specularColor, F);
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
    float3 total = (balancedDiff * albedoColor + specular) * Attenuate(light, worldPos) * light.intensity * light.Color;
    return total;
}

float3 PBRSpotLight(Light light, float3 normal, float3 worldPos, float3 toCamera, float3 albedoColor, float3 specular, float roughness, float metalness)
{
    float3 toPixel = normalize(worldPos - light.Position);
    float pixelAngle = saturate(dot(toPixel, -normalize(light.Direction)));
    float cosOuter = cos(radians(light.SpotOuterAngle));
    float cosInner = cos(radians(light.SpotInnerAngle));
    float falloffRange = cosOuter - cosInner;
    
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    float3 finalColor = PBRPointLight(light, normal, worldPos, albedoColor, toCamera, specular, roughness, metalness) * spotTerm;
    return finalColor;
}

float4 PBRCalculateLights(Texture2D Albedo, Texture2D RoughnessMap, Texture2D MetalnessMap, SamplerState Sampler, VTP_Normal input, Light lights[MAX_LIGHTS],
    int lightCount, float3 cameraPosition, float3 normal, float shadowAmount)
{
    float4 finalColor = float4(0, 0, 0, 1);

    float4 albedoColor = Albedo.Sample(Sampler, input.uv);
    albedoColor.rgb = pow(albedoColor.rgb, 2.2f);
    float roughness = RoughnessMap.Sample(Sampler, input.uv).r;
    float metalness = MetalnessMap.Sample(Sampler, input.uv).r;
    float3 specularColor = lerp(0.04f, albedoColor.rgb, metalness);
    
    for (int i = 0; i < lightCount; i++)
    {
        float4 total = float4(0, 0, 0, 1);
        float3 toLight = normalize(input.worldPosition - lights[i].Position);
        float3 toCamera = normalize(cameraPosition - input.worldPosition);

        Light light = lights[i];
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                finalColor.rgb += PBRDirectionalLight(light, normal, albedoColor.rgb, toCamera, specularColor, roughness, metalness);
                break;
            case LIGHT_TYPE_POINT:
                finalColor.rgb += PBRPointLight(light, normal, input.worldPosition, albedoColor.rgb, toCamera, specularColor, roughness, metalness);
                break;
            case LIGHT_TYPE_SPOT:
                finalColor.rgb += PBRSpotLight(light, normal, input.worldPosition, toCamera, albedoColor.rgb, specularColor, roughness, metalness);
                break;
            default:
                break;
        }
        
        if(i == 0)
        {
            finalColor.rgb *= shadowAmount;
        }
    }
    
    return finalColor;
}

float4 PBRCalculateLights(Texture2D Albedo, Texture2D RoughnessMap, Texture2D MetalnessMap, SamplerState Sampler, VertexToPixel input, Light lights[MAX_LIGHTS],
    int lightCount, float3 cameraPosition, float3 normal, float shadowAmount)
{
    VTP_Normal normalInput;
    normalInput.normal = input.normal;
    normalInput.uv = input.uv;
    normalInput.worldPosition = input.worldPosition;
    normalInput.screenPosition = input.screenPosition;
    return PBRCalculateLights(Albedo, RoughnessMap, MetalnessMap, Sampler, normalInput, lights, lightCount, cameraPosition, normal, shadowAmount);
}
#endif