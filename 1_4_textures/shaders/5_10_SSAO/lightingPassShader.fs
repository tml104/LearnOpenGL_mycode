#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssaoColor;

const int NR_LIGHTS = 1;

struct Light{
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    //float Radius;
};

uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

float GetAmbientStrength(vec2 TexCoords)
{
    //ambient
    return texture(ssaoColor, TexCoords).r;
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

    // [debug]
    //FragColor = vec4(vec3(Diffuse), 1.0);
    //FragColor = vec4(vec3(Specular), 1.0);
    //FragColor = vec4(vec3(texture(ssaoColor, TexCoords).r), 1.0);

    //ambient (low cost)
    //vec3 res = vec3(0.0);
    vec3 res = 0.3 * GetAmbientStrength(TexCoords) * Diffuse;
    //vec3 res = GetAmbientStrength(TexCoords) * vec3(0.95);

    for(int i=0;i<NR_LIGHTS; i++)
    {

        float attenuation = GetAttenuation(FragPos, lights[i].Position, lights[i].Linear, lights[i].Quadratic);
        // diffuse
        res += GetDiffuseStrength(Normal, FragPos, lights[i].Position, lights[i].Color) * Diffuse ;
        // specular
        res += GetSpecularStrength(Normal, FragPos, lights[i].Position, lights[i].Color, viewPos) * Specular ;

    }

    // const float gamma = 2.2;
    // vec3 result = vec3(1.0) - exp(-res * 0.5);
    // //result = pow(result, vec3(1.0 / gamma));
    // FragColor = vec4(result, 1.0);

    FragColor = vec4(res, 1.0);

}