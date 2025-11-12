#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

out vec3 v_Color;
out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
    // Transform vertex position by model matrix first, then view-projection
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    gl_Position = u_ViewProjection * worldPos;
    v_Color = a_Color;
    v_FragPos = vec3(worldPos);
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
}

