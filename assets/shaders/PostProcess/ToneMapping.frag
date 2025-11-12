#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_HDRTexture;
uniform float u_Exposure;
uniform int u_ToneMapperType; // 0 = ACES, 1 = Hable, 2 = Reinhard, 3 = None

// ACES Filmic Tone Mapping (approximate)
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// ACES Filmic Tone Mapping (more accurate)
vec3 ACESFilm_Accurate(vec3 x) {
    x *= 0.6; // Pre-exposure adjustment
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Hable/Uncharted 2 Tone Mapping
vec3 Uncharted2Tonemap(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 HableTonemap(vec3 color) {
    const float W = 11.2;
    color = Uncharted2Tonemap(color * 2.0);
    vec3 whiteScale = 1.0 / Uncharted2Tonemap(vec3(W));
    return color * whiteScale;
}

// Reinhard Tone Mapping
vec3 ReinhardTonemap(vec3 color) {
    return color / (1.0 + color);
}

void main() {
    // Sample HDR texture
    vec3 hdrColor = texture(u_HDRTexture, v_TexCoord).rgb;
    
    // Apply exposure
    vec3 exposedColor = hdrColor * u_Exposure;
    
    // Apply tone mapping
    vec3 toneMappedColor;
    if (u_ToneMapperType == 0) {
        // ACES
        toneMappedColor = ACESFilm(exposedColor);
    } else if (u_ToneMapperType == 1) {
        // Hable/Uncharted 2
        toneMappedColor = HableTonemap(exposedColor);
    } else if (u_ToneMapperType == 2) {
        // Reinhard
        toneMappedColor = ReinhardTonemap(exposedColor);
    } else {
        // None (clamp only)
        toneMappedColor = clamp(exposedColor, 0.0, 1.0);
    }
    
    // Gamma correction (sRGB)
    vec3 finalColor = pow(toneMappedColor, vec3(1.0 / 2.2));
    
    FragColor = vec4(finalColor, 1.0);
}

