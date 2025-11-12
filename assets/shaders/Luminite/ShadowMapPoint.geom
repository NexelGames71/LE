#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_LightViewProjections[6]; // One for each cubemap face
uniform vec3 u_LightPosition;

in vec3 v_WorldPos[];

out vec4 v_FragPos;

void main() {
    // Render to all 6 faces of the cubemap
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face; // Select cubemap face
        
        for (int i = 0; i < 3; ++i) {
            v_FragPos = vec4(v_WorldPos[i], 1.0);
            gl_Position = u_LightViewProjections[face] * v_FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}

