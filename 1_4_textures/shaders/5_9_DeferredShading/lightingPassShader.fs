#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

const int NR_LIGHTS = 32;

struct Light{
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
};

uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

float GetAmbientStrength()
{
    //ambient
    return 0.2;
}

vec3 GetDiffuseStrength(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    return diffuse;
}

vec3 GetSpecularStrength(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor, vec3 viewPos)
{
    // specular
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    

    return specular;
}

float GetAttenuation(vec3 fragPos, vec3 lightPos, float linear, float quadratic)
{
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + linear * distance +  quadratic * distance * distance);

    return attenuation;
}


void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Diffuse = AlbedoSpec.rgb;
    float Specular = AlbedoSpec.a;

    //FragColor = vec4(vec3(Specular), 1.0);

    //ambient
    vec3 res = GetAmbientStrength() * Diffuse;
    for(int i=0;i<NR_LIGHTS; i++)
    {
        float attenuation = GetAttenuation(FragPos, lights[i].Position, lights[i].Linear, lights[i].Quadratic);
        // diffuse
        res += GetDiffuseStrength(Normal, FragPos, lights[i].Position, lights[i].Color) * Diffuse * attenuation;
        // specular
        res += GetSpecularStrength(Normal, FragPos, lights[i].Position, lights[i].Color, viewPos) * Specular * attenuation;
    }

    FragColor = vec4(res, 1.0);

}