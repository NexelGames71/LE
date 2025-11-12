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

// Constants
const float PI = 3.14159265359;

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
    
    // Light direction (directional light)
    vec3 L = normalize(-u_LightDirection);
    
    // Calculate lighting
    vec3 albedo = baseColor;
    vec3 Lo = CookTorranceBRDF(N, V, L, albedo, metalness, roughness);
    
    // Apply light color and intensity
    Lo *= u_LightColor * u_LightIntensity * max(dot(N, L), 0.0);
    
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


