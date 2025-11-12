#version 330 core

in vec3 v_Color;
in vec3 v_Normal;
in vec3 v_FragPos;

out vec4 FragColor;

uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform vec3 u_ViewPos;
uniform vec3 u_MaterialColor;  // Material base color
uniform int u_UseVertexColor;  // Whether to use vertex color or material color

void main()
{
    // Use material color or vertex color based on uniform
    vec3 baseColor = u_UseVertexColor > 0 ? v_Color : u_MaterialColor;
    
    // Ambient lighting (moderate for natural lighting)
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * u_LightColor * u_LightIntensity;
    
    // Diffuse lighting
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(-u_LightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor * u_LightIntensity;
    
    // Specular lighting (moderate shine)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * u_LightColor * u_LightIntensity;
    
    // Combine lighting with material/vertex color
    vec3 litColor = (ambient + diffuse + specular) * baseColor;
    
    // Clamp to prevent overexposure
    litColor = min(litColor, vec3(1.0));
    
    FragColor = vec4(litColor, 1.0);
}

