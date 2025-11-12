// Luminite Lighting System - Common shader code
// This file contains shared definitions and functions for lighting calculations

#ifndef LUMINITE_LIGHTING_COMMON_GLSL
#define LUMINITE_LIGHTING_COMMON_GLSL

// Light types
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define LIGHT_TYPE_AREA 3
#define LIGHT_TYPE_SKY 4

// Feature flags
#define FEATURE_LUMA_LIT 1
#define FEATURE_LUMA_IBL 2
#define FEATURE_LUMA_SHADOWS 4
#define FEATURE_LUMA_CSM 8
#define FEATURE_LUMA_REFLECTION_PROBES 16
#define FEATURE_LUMA_AREA_LIGHTS 32

// Light structure (matches LightGPU)
struct Light {
    vec3 position;
    float _padding0;
    vec3 direction;
    float _padding1;
    vec3 color;
    float intensity;
    float range;
    float innerCone;
    float outerCone;
    float _padding2;
    uint type;
    uint flags;
    float _padding3;
    float _padding4;
};

// Frame lighting UBO (matches FrameLightingUBO)
layout(std140) uniform FrameLighting {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    float exposure;
    vec3 ambientColor;
    float ambientIntensity;
    uint featureFlags;
    float _padding0;
    float _padding1;
    float _padding2;
    uint directionalLightCount;
    uint pointLightCount;
    uint spotLightCount;
    uint totalLightCount;
    float shadowBias;
    float shadowNormalBias;
    float shadowDistance;
    float _padding3;
} u_FrameLighting;

// Placeholder lighting calculation functions
vec3 CalculateDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * NdotL;
    return diffuse * albedo;
}

vec3 CalculatePointLight(Light light, vec3 position, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - position);
    float distance = length(light.position - position);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    
    if (distance > light.range) {
        return vec3(0.0);
    }
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * NdotL * attenuation;
    return diffuse * albedo;
}

vec3 CalculateSpotLight(Light light, vec3 position, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - position);
    float distance = length(light.position - position);
    
    if (distance > light.range) {
        return vec3(0.0);
    }
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCone - light.outerCone;
    float intensity = clamp((theta - light.outerCone) / epsilon, 0.0, 1.0);
    
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * NdotL * attenuation * intensity;
    return diffuse * albedo;
}

#endif // LUMINITE_LIGHTING_COMMON_GLSL

