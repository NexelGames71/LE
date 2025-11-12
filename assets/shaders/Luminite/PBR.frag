#version 330 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
in mat3 v_TBN;

out vec4 FragColor;

// Material textures
uniform sampler2D u_BaseColorTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AOTexture;
uniform sampler2D u_EmissiveTexture;

// Material properties
uniform vec3 u_BaseColor;
uniform float u_Metalness;
uniform float u_Roughness;
uniform float u_NormalScale;
uniform float u_AO;
uniform vec3 u_Emissive;
uniform float u_EmissiveIntensity;

// Texture flags
uniform bool u_HasBaseColorTexture;
uniform bool u_HasMetalnessTexture;
uniform bool u_HasRoughnessTexture;
uniform bool u_HasNormalTexture;
uniform bool u_HasAOTexture;
uniform bool u_HasEmissiveTexture;

// Lighting
uniform vec3 u_CameraPos;
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;

// Exposure and tonemapping
uniform float u_Exposure;
uniform bool u_UseTonemapping;

// IBL (Image-Based Lighting)
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;
uniform float u_IBLIntensity;
uniform bool u_UseIBL;

// Shadow mapping
uniform sampler2D u_ShadowMap;
uniform mat4 u_LightViewProjection;
uniform float u_ShadowBias;
uniform float u_ShadowNormalBias;
uniform bool u_CastShadows;
uniform int u_PCFSize;

// Constants
const float PI = 3.14159265359;

// Physical Lighting Functions
// Convert directional light intensity (lux) to radiance
float LuxToRadiance(float lux) {
    const float SUN_SOLID_ANGLE = 6.8e-5; // steradians
    return lux / SUN_SOLID_ANGLE;
}

// Calculate directional light contribution with proper physical units
vec3 CalculateDirectionalLightPhysical(
    vec3 lightDirection,
    vec3 lightColor,
    float lightIntensityLux,
    vec3 N,
    vec3 V,
    vec3 albedo,
    float metalness,
    float roughness
) {
    vec3 L = normalize(-lightDirection);
    float NdotL = max(dot(N, L), 0.0);
    
    if (NdotL <= 0.0) {
        return vec3(0.0);
    }
    
    // Convert lux to radiance
    float radiance = LuxToRadiance(lightIntensityLux);
    
    // Calculate BRDF (Cook-Torrance with GGX)
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    // NDF (GGX/Trowbridge-Reitz)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    float NDF = num / max(denom, 0.0001);
    
    // Geometry (Smith with Schlick-GGX)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    float G = ggx1 * ggx2;
    
    // Fresnel (Schlick)
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
    
    // Specular BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Energy conservation
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metalness);
    
    // Diffuse BRDF (Lambertian)
    vec3 diffuse = kD * albedo / PI;
    
    // Combine and apply radiance
    vec3 brdf = diffuse + specular;
    return brdf * lightColor * radiance * NdotL;
}

// Shadow sampling functions
float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    if (!u_CastShadows) {
        return 1.0;
    }
    
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0) {
        return 1.0;
    }
    
    // Get depth from shadow map
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
    
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Apply bias to prevent shadow acne
    float bias = u_ShadowBias;
    bias += u_ShadowNormalBias * (1.0 - abs(dot(normal, lightDir)));
    
    // Check if current fragment is in shadow
    float shadow = currentDepth - bias > closestDepth ? 0.0 : 1.0;
    
    // PCF (Percentage Closer Filtering) for soft shadows
    if (u_PCFSize > 0) {
        shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
        int pcfRadius = u_PCFSize / 2;
        
        for (int x = -pcfRadius; x <= pcfRadius; ++x) {
            for (int y = -pcfRadius; y <= pcfRadius; ++y) {
                float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        shadow /= float((u_PCFSize * u_PCFSize));
    }
    
    return shadow;
}

// ACES Tonemapping (approximate)
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Schlick Fresnel approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel with roughness (for IBL)
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

// Smith Geometry Function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Cook-Torrance BRDF
vec3 CookTorranceBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metalness, float roughness) {
    vec3 H = normalize(V + L);
    
    // Calculate F0 (base specular reflectance)
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    // Cook-Torrance BRDF components
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    // Specular BRDF
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Energy conservation: kS = F, kD = 1.0 - kS
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;  // Dielectrics have diffuse, metals don't
    
    // Diffuse BRDF (Lambertian)
    vec3 diffuse = kD * albedo / PI;
    
    return diffuse + specular;
}

void main() {
    // Sample material textures
    vec3 baseColor = u_HasBaseColorTexture ? texture(u_BaseColorTexture, v_TexCoord).rgb : u_BaseColor;
    float metalness = u_HasMetalnessTexture ? texture(u_MetalnessTexture, v_TexCoord).r : u_Metalness;
    float roughness = u_HasRoughnessTexture ? texture(u_RoughnessTexture, v_TexCoord).r : u_Roughness;
    float ao = u_HasAOTexture ? texture(u_AOTexture, v_TexCoord).r : u_AO;
    vec3 emissive = u_HasEmissiveTexture ? texture(u_EmissiveTexture, v_TexCoord).rgb : u_Emissive;
    
    // Sample and apply normal map
    vec3 N = v_Normal;
    if (u_HasNormalTexture) {
        vec3 normalMap = texture(u_NormalTexture, v_TexCoord).rgb * 2.0 - 1.0;
        normalMap.xy *= u_NormalScale;
        N = normalize(v_TBN * normalMap);
    }
    
    // View direction
    vec3 V = normalize(u_CameraPos - v_WorldPos);
    
    // Calculate lighting with physical units
    vec3 albedo = baseColor;
    
    // Use physical lighting calculation for directional light
    // u_LightIntensity is in lux for directional lights
    // Typical values: 100-1000 lux (indoor), 10000-100000 lux (outdoor/sun)
    // For now, always use physical units (lux → radiance conversion)
    // Typical sun intensity: ~100,000 lux
    float lightIntensityLux = u_LightIntensity;
    
    // Use physical lighting calculation (lux → radiance conversion)
    vec3 Lo = CalculateDirectionalLightPhysical(
        u_LightDirection,
        u_LightColor,
        lightIntensityLux,  // lux
        N, V, albedo, metalness, roughness
    );
    
    // Apply shadows
    float shadow = 1.0;
    if (u_CastShadows) {
        // Calculate fragment position in light space
        vec4 fragPosLightSpace = u_LightViewProjection * vec4(v_WorldPos, 1.0);
        vec3 lightDir = normalize(-u_LightDirection);
        shadow = CalculateShadow(fragPosLightSpace, N, lightDir);
    }
    
    // Apply shadow to lighting
    Lo *= shadow;
    
    // IBL (Image-Based Lighting)
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 ambient = vec3(0.0);
    
    if (u_UseIBL) {
        // Sample irradiance map for diffuse IBL
        vec3 irradiance = texture(u_IrradianceMap, N).rgb;
        vec3 diffuseIBL = irradiance * albedo;
        
        // Sample prefilter map and BRDF LUT for specular IBL
        vec3 R = reflect(-V, N);
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        vec3 specularIBL = prefilteredColor * (F * brdf.x + brdf.y);
        
        // Combine diffuse and specular IBL
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;
        ambient = (kD * diffuseIBL + specularIBL) * u_IBLIntensity;
    } else {
        // Fallback to simple ambient
        ambient = u_AmbientColor * u_AmbientIntensity * albedo;
    }
    
    // Apply AO
    Lo = Lo * ao + ambient;
    
    // Add emissive
    Lo += emissive * u_EmissiveIntensity;
    
    // Apply exposure
    vec3 color = Lo * u_Exposure;
    
    // Tonemapping (ACES)
    if (u_UseTonemapping) {
        color = ACESFilm(color);
    }
    
    // Gamma correction (sRGB)
    color = pow(color, vec3(1.0 / 2.2));
    
    FragColor = vec4(color, 1.0);
}


