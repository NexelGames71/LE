#version 330 core

in vec4 v_FragPos;

uniform vec3 u_LightPosition;
uniform float u_FarPlane;

void main() {
    // Calculate distance from light
    float lightDistance = length(v_FragPos.xyz - u_LightPosition);
    
    // Normalize to [0,1] range
    lightDistance = lightDistance / u_FarPlane;
    
    // Write to depth
    gl_FragDepth = lightDistance;
}

