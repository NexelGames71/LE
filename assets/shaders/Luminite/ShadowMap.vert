#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_LightViewProjection;
uniform mat4 u_ModelMatrix;

void main() {
    vec4 worldPos = u_ModelMatrix * vec4(a_Position, 1.0);
    gl_Position = u_LightViewProjection * worldPos;
}

