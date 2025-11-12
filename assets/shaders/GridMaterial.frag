#version 330 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec3 v_Color;

out vec4 FragColor;

uniform vec3 u_GridColor1;
uniform vec3 u_GridColor2;
uniform float u_GridSize;
uniform float u_GridThickness;
uniform vec3 u_BaseColor;
uniform float u_GridIntensity;
uniform vec3 u_ViewPos;

// Grid material similar to Unreal Engine's grid material
void main() {
    vec3 worldPos = v_WorldPos;
    
    // Calculate grid lines
    vec2 gridCoord = worldPos.xz / u_GridSize;
    
    // Get fractional part to find distance to grid lines
    vec2 gridFract = fract(gridCoord);
    
    // Distance to nearest grid line (both directions)
    vec2 distToLine = min(gridFract, 1.0 - gridFract);
    float minDist = min(distToLine.x, distToLine.y);
    
    // Create grid lines with smoothstep for anti-aliased lines
    float lineWidth = u_GridThickness;
    float gridMask = 1.0 - smoothstep(0.0, lineWidth, minDist);
    
    // Mix colors - bright lines on dark background
    vec3 finalColor = mix(u_BaseColor, u_GridColor2, gridMask);
    
    // Apply intensity
    finalColor *= u_GridIntensity;
    
    // Ensure minimum brightness so grid is always visible
    finalColor = max(finalColor, u_BaseColor);
    
    FragColor = vec4(finalColor, 1.0);
}


