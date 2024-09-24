#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform bool normalMapping;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewPos)
{
    vec3 lightColor = vec3(1.0);
    //ambient
    vec3 ambient = 0.3 * lightColor;

    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    // simple attenuation
    float max_distance = 1.5; //no use??
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (distance);
    
    // ambient *= attenuation;
    // diffuse *= attenuation;
    // specular *= attenuation;
    

    return ambient + diffuse + specular;
}

void main()
{           
    // Obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // Transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space


    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    vec3 res = BlinnPhong(normal, fs_in.TangentFragPos, fs_in.TangentLightPos, fs_in.TangentViewPos) * color; // don't forget to normalize normal!

    if(normalMapping)
    {
        FragColor = vec4(res, 1.0);
    }
    else{
        FragColor = vec4(texture(diffuseMap, fs_in.TexCoords).rgb, 1.0);
    }
}