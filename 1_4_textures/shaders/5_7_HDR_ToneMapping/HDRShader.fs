#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}fs_in;

uniform sampler2D diffuseMap;

uniform vec3 lightPosList[4];
uniform vec3 lightColorList[4];
uniform vec3 viewPos;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor, vec3 viewPos)
{
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
    vec3 specular = spec * lightColor ;    

    // simple attenuation
    //float max_distance = 1.5; //no use??
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (distance * distance);
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    

    return ambient + diffuse + specular;
}

void main()
{
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    vec3 res = vec3(0.0);
    for(int i=0;i<4;i++)
    {
        res += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPosList[i], lightColorList[i], viewPos) * color; // don't forget to normalize normal!
    }

    FragColor = vec4(res, 1.0);
}