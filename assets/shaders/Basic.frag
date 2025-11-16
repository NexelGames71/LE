#version 430 core

in vec3 v_Color;
in vec3 v_Normal;
in vec3 v_FragPos;

out vec4 FragColor;

uniform vec3 u_ViewPos;
uniform vec3 u_MaterialColor;  // Material base color
uniform int u_UseVertexColor;  // Whether to use vertex color or material color
uniform int u_LightCount;      // Number of active lights

// Shadow mapping
uniform sampler2D u_DirectionalShadowMap;
uniform mat4 u_LightViewProj;
uniform int u_HasDirectionalShadow;  // 1 if shadow map is valid, 0 otherwise

// Light data structure (matches C++ LightDataGPU)
struct LightDataGPU {
    vec4 Position;      // w can be 1.0 for point, 0 for directional
    vec4 Direction;     // normalized, w unused
    vec4 ColorIntensity; // rgb = color * intensity, a = intensity
    float Range;         // point/spot
    float InnerCone;     // radians
    float OuterCone;     // radians
    int Type;            // 0 = Directional, 1 = Point, 2 = Spot
    int CastShadows;     // bool, but as int for GPU
};

// Light buffer (SSBO)
layout(std430, binding = 3) readonly buffer LightsBuffer {
    LightDataGPU u_Lights[];
};

// Shadow calculation
float ComputeShadow(vec3 worldPos) {
    if (u_HasDirectionalShadow == 0) {
        return 1.0; // No shadow
    }
    
    vec4 lightSpacePos = u_LightViewProj * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5; // [-1,1] -> [0,1]
    
    // Early out if outside shadow map
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }
    
    float closestDepth = texture(u_DirectionalShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.001;
    
    float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;
    
    // PCF (Percentage-Closer Filtering) for softer shadows
    float shadowSum = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_DirectionalShadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_DirectionalShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadowSum += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
        }
    }
    shadow = shadowSum / 9.0;
    
    return shadow;
}

// PBR lighting calculation
vec3 ApplyLight(LightDataGPU light, vec3 worldPos, vec3 normal, vec3 albedo, float roughness, float metallic) {
    vec3 N = normalize(normal);
    vec3 V = normalize(u_ViewPos - worldPos);
    
    vec3 L;
    float attenuation = 1.0;
    
    if (light.Type == 0) {
        // Directional light
        L = normalize(-light.Direction.xyz);
    } else {
        // Point or Spot light
        vec3 lightPos = light.Position.xyz;
        vec3 toLight = lightPos - worldPos;
        float dist = length(toLight);
        L = toLight / dist;
        
        // Simple falloff
        float d = dist / light.Range;
        attenuation = clamp(1.0 - d * d, 0.0, 1.0);
        
        // Spot light cone
        if (light.Type == 2) {
            float theta = dot(L, normalize(-light.Direction.xyz));
            float epsilon = light.InnerCone - light.OuterCone;
            float spotIntensity = clamp((theta - light.OuterCone) / epsilon, 0.0, 1.0);
            attenuation *= spotIntensity;
        }
    }
    
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    
    if (NdotL <= 0.0) {
        return vec3(0.0);
    }
    
    // Get radiance from light
    vec3 radiance = light.ColorIntensity.rgb * attenuation;
    
    // Simple PBR approximation (Cook-Torrance BRDF simplified)
    // For now, use a simple diffuse + specular model
    // Full PBR would use GGX, Smith, Fresnel, etc.
    
    // Diffuse (Lambert)
    vec3 diffuse = albedo / 3.14159; // Lambertian BRDF
    
    // Specular (Blinn-Phong approximation)
    float NdotH = max(dot(N, H), 0.0);
    float specPower = (1.0 - roughness) * 128.0; // Convert roughness to shininess
    float spec = pow(NdotH, specPower);
    vec3 specular = vec3(spec) * (1.0 - metallic) * 0.5; // Mix with metallic
    
    // Combine
    return (diffuse + specular) * radiance * NdotL;
}

void main()
{
    // Use material color or vertex color based on uniform
    vec3 albedo = u_UseVertexColor > 0 ? v_Color : u_MaterialColor;
    
    // Simple material properties (can be made into uniforms later)
    float roughness = 0.5;
    float metallic = 0.0;
    
    vec3 normal = normalize(v_Normal);
    vec3 worldPos = v_FragPos;
    
    // Calculate shadow factor (only for directional lights)
    float shadowFactor = ComputeShadow(worldPos);
    
    // Accumulate lighting from all lights
    vec3 lighting = vec3(0.0);
    
    for (int i = 0; i < u_LightCount && i < 64; ++i) {
        vec3 lightContrib = ApplyLight(u_Lights[i], worldPos, normal, albedo, roughness, metallic);
        
        // Apply shadow only to directional lights
        if (u_Lights[i].Type == 0) {
            lightContrib *= shadowFactor;
        }
        
        lighting += lightContrib;
    }
    
    // Add ambient
    vec3 ambient = vec3(0.03) * albedo;
    lighting += ambient;
    
    // Combine lighting with albedo
    vec3 litColor = lighting;
    
    // Tone mapping (simple Reinhard)
    litColor = litColor / (litColor + vec3(1.0));
    
    // Gamma correction
    litColor = pow(litColor, vec3(1.0 / 2.2));
    
    FragColor = vec4(litColor, 1.0);
}
