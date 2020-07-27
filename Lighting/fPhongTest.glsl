//#version 330 core
#version 140

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 LightPos;
uniform vec3 ViewPos;
uniform bool blinn;

uniform bool bAmbieni;
uniform bool bDiffuse;
uniform bool bSpecular;

void main()
{    
    vec3 color = Color;
    // ambient
    vec3 ambient = 0.6 * color;
    // diffuse
    vec3 lightDir = normalize(LightPos - FragPos);
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
//    vec3 diffuse = diff * color;
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(ViewPos - FragPos);
    // vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }

    vec3 specular = vec3(0.5) * spec; // assuming bright white light color
    
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    if(bAmbieni)
    {
        FragColor.xyz += ambient;
    }

    if(bDiffuse)
    {
        FragColor.xyz += diffuse;
    }

    if(bSpecular)
    {
        FragColor.xyz += specular;
    }

//    FragColor = vec4(ambient + diffuse + specular, 1.0);

//    FragColor =  vec4(lightDir.x, lightDir.y, lightDir.z, 1);
//    FragColor =  vec4(normal.x, normal.y, normal.z, 1);
}
