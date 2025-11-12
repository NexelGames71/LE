// Physical Lighting Functions
// Proper unit conversions for physically-based lighting

#ifndef PHYSICAL_LIGHTING_GLSL
#define PHYSICAL_LIGHTING_GLSL

const float PI = 3.14159265359;

// Convert directional light intensity (lux) to radiance
// lux = illuminance (lm/m²), we need radiance (W/(sr·m²))
// For directional lights, we approximate the sun's solid angle
// Typical sun solid angle: ~6.8e-5 steradians
float LuxToRadiance(float lux) {
    // Approximate conversion: radiance ≈ illuminance / solid_angle
    // For a directional light representing the sun:
    const float SUN_SOLID_ANGLE = 6.8e-5; // steradians
    return lux / SUN_SOLID_ANGLE;
}

// Convert point light intensity (lumens) to luminous intensity (candela)
// For a point light, luminous intensity = lumens / solid_angle
// Full sphere solid angle = 4π
float LumensToCandela(float lumens) {
    return lumens / (4.0 * PI);
}

// Convert spot light intensity (lumens) to luminous intensity (candela)
// For a spot light, luminous intensity = lumens / solid_angle_of_cone
// Solid angle of cone = 2π(1 - cos(θ/2))
float LumensToCandelaSpot(float lumens, float outerConeAngle) {
    float halfAngle = outerConeAngle * 0.5;
    float solidAngle = 2.0 * PI * (1.0 - cos(halfAngle));
    return lumens / solidAngle;
}

// Calculate radiance from luminous intensity (candela) and distance
// For point/spot lights: radiance = luminous_intensity / distance²
float CandelaToRadiance(float candela, float distance) {
    return candela / (distance * distance);
}

// Calculate directional light contribution with proper physical units
vec3 CalculateDirectionalLightPhysical(
    vec3 lightDirection,
    vec3 lightColor,
    float lightIntensityLux,  // Intensity in lux
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
    
    // Calculate BRDF (Cook-Torrance)
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    // NDF (GGX)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    float NDF = num / max(denom, 0.0001);
    
    // Geometry (Smith)
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

// Calculate point light contribution with proper physical units
vec3 CalculatePointLightPhysical(
    vec3 lightPosition,
    vec3 lightColor,
    float lightIntensityLumens,  // Intensity in lumens
    float lightRange,             // Range in meters
    vec3 worldPos,
    vec3 N,
    vec3 V,
    vec3 albedo,
    float metalness,
    float roughness
) {
    vec3 L = lightPosition - worldPos;
    float distance = length(L);
    
    // Check if within range
    if (distance > lightRange) {
        return vec3(0.0);
    }
    
    L = normalize(L);
    float NdotL = max(dot(N, L), 0.0);
    
    if (NdotL <= 0.0) {
        return vec3(0.0);
    }
    
    // Convert lumens to candela, then to radiance
    float candela = LumensToCandela(lightIntensityLumens);
    float radiance = CandelaToRadiance(candela, distance);
    
    // Apply inverse square law falloff
    float attenuation = 1.0 / (distance * distance);
    
    // Calculate BRDF (same as directional)
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    // NDF (GGX)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    float NDF = num / max(denom, 0.0001);
    
    // Geometry (Smith)
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
    return brdf * lightColor * radiance * NdotL * attenuation;
}

// Calculate spot light contribution with proper physical units
vec3 CalculateSpotLightPhysical(
    vec3 lightPosition,
    vec3 lightDirection,
    vec3 lightColor,
    float lightIntensityLumens,  // Intensity in lumens
    float lightRange,             // Range in meters
    float innerConeCos,          // Cosine of inner cone angle
    float outerConeCos,          // Cosine of outer cone angle
    vec3 worldPos,
    vec3 N,
    vec3 V,
    vec3 albedo,
    float metalness,
    float roughness
) {
    vec3 L = lightPosition - worldPos;
    float distance = length(L);
    
    // Check if within range
    if (distance > lightRange) {
        return vec3(0.0);
    }
    
    L = normalize(L);
    float NdotL = max(dot(N, L), 0.0);
    
    if (NdotL <= 0.0) {
        return vec3(0.0);
    }
    
    // Calculate spot light cone attenuation
    vec3 lightDir = normalize(-lightDirection);
    float cosAngle = dot(L, lightDir);
    float spotFactor = smoothstep(outerConeCos, innerConeCos, cosAngle);
    
    if (spotFactor <= 0.0) {
        return vec3(0.0);
    }
    
    // Convert lumens to candela (using outer cone angle)
    float outerConeAngle = acos(outerConeCos);
    float candela = LumensToCandelaSpot(lightIntensityLumens, outerConeAngle);
    float radiance = CandelaToRadiance(candela, distance);
    
    // Apply inverse square law falloff
    float attenuation = 1.0 / (distance * distance);
    
    // Calculate BRDF (same as directional/point)
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    
    // NDF (GGX)
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    float NDF = num / max(denom, 0.0001);
    
    // Geometry (Smith)
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
    return brdf * lightColor * radiance * NdotL * attenuation * spotFactor;
}

#endif // PHYSICAL_LIGHTING_GLSL

