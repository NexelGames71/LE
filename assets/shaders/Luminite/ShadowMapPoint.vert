#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ModelMatrix;

out vec3 v_WorldPos;

void main() {
    vec4 worldPos = u_ModelMatrix * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    gl_Position = worldPos; // Will be transformed in geometry shader
}

