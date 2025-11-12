#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform float u_SkyboxScale;

out vec3 v_TexCoord;

void main()
{
    v_TexCoord = a_Position; // Use original position for texture coordinates
    vec4 pos = u_ViewProjection * vec4(a_Position * u_SkyboxScale, 1.0);
    gl_Position = pos.xyww; // Ensure depth is always 1.0 (furthest)
}


