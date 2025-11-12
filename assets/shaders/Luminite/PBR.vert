#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
out mat3 v_TBN;  // Tangent-Bitangent-Normal matrix for normal mapping

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_ViewProjection;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_TexCoord = a_TexCoord;
    
    // Calculate TBN matrix for normal mapping
    vec3 N = normalize(mat3(u_Model) * a_Normal);
    vec3 T = normalize(mat3(u_Model) * a_Tangent);
    T = normalize(T - dot(T, N) * N);  // Gram-Schmidt orthogonalization
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);
    
    v_Normal = N;
    
    gl_Position = u_ViewProjection * worldPos;
}













