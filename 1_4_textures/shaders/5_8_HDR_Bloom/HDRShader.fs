#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}fs_in;

uniform sampler2D diffuseMap;

uniform vec3 lightPosList[4];
uniform vec3 lightColorList[4];
uniform vec3 viewPos;

uniform float bloomThreshold;


vec3 GetAmbient(vec3 color)
{
    //ambient
    return 0.0 * color;
}

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor, vec3 viewPos)
{
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
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    //return diffuse + specular;
    return diffuse;
}

void main()
{
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    vec3 res = GetAmbient(color);
    for(int i=0;i<4;i++)
    {
        res += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPosList[i], lightColorList[i], viewPos) * color; // don't forget to normalize normal!
    }

    float brightness = dot(res, bloomThreshold*vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
    {
        BrightColor = vec4(res, 1.0);
    }
    else
    {
        BrightColor = vec4(vec3(0.0), 1.0);
    }

    FragColor = vec4(res, 1.0);
}