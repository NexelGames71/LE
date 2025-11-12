#version 330 core

in vec3 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_SkyboxTexture;
uniform int u_UseTexture;
uniform float u_CloudOffset;

// Convert 3D direction to equirectangular UV coordinates
vec2 DirectionToUV(vec3 dir) {
    float u = atan(dir.z, dir.x) / (2.0 * 3.14159265359) + 0.5;
    float v = acos(dir.y) / 3.14159265359;
    return vec2(u, v);
}

void main()
{
    if (u_UseTexture == 1) {
        // Sample from HDR equirectangular texture with horizontal offset for cloud movement
        vec2 uv = DirectionToUV(normalize(v_TexCoord));
        // Add horizontal offset to U coordinate for pure horizontal cloud movement
        // Use fract to wrap around seamlessly (avoids circular appearance)
        uv.x = fract(uv.x + u_CloudOffset);
        vec3 color = texture(u_SkyboxTexture, uv).rgb;
        
        // Apply higher exposure for bright sky (doesn't affect object lighting)
        float exposure = 2.0;
        color *= exposure;
        
        // Tone mapping (Reinhard with white point)
        vec3 mapped = color / (color + vec3(1.0));
        
        // Moderate saturation
        float luminance = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
        vec3 saturated = mix(vec3(luminance), mapped, 1.2);
        
        // Brightness boost for sky (independent of object lighting)
        saturated = saturated * 1.3;
        
        // Clamp to prevent overexposure
        saturated = min(saturated, vec3(1.0));
        
        FragColor = vec4(saturated, 1.0);
    } else {
        // Fallback gradient (simple blue sky)
        float t = (v_TexCoord.y + 1.0) * 0.5; // Normalize y from [-1,1] to [0,1]
        vec3 topColor = vec3(0.5, 0.7, 1.0);
        vec3 bottomColor = vec3(0.8, 0.9, 1.0);
        vec3 color = mix(bottomColor, topColor, t);
        FragColor = vec4(color, 1.0);
    }
}

