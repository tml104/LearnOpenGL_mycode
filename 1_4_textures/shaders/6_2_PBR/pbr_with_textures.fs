#version 420 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 TexCoords;
} fs_in;

// uniform sampler2D ballTexture;

// uniform vec3 albedo;
// uniform float metallic;
// uniform float roughness;
// uniform float ao;
layout (binding = 0) uniform sampler2D albedo_texture;
layout (binding = 1) uniform sampler2D normal_texture;
layout (binding = 2) uniform sampler2D metallic_texture;
layout (binding = 3) uniform sampler2D roughness_texture;
layout (binding = 4) uniform sampler2D ao_texture;

// lights
uniform vec3 lightPos[4];
uniform vec3 lightColors[4];

uniform vec3 viewPos;

const float PI = 3.14159265359;

// NDF (D)
float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0; // 所以这里还是做了重映射

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 自带gamma矫正？？？
    vec3 albedo = pow(texture(albedo_texture, fs_in.TexCoords).rgb, vec3(2.2));
    float metallic = texture(metallic_texture, fs_in.TexCoords).r;
    float roughness = texture(roughness_texture, fs_in.TexCoords).r;
    float ao = texture(ao_texture, fs_in.TexCoords).r;

    vec3 N = normalize(fs_in.WorldNormal);
    vec3 V = normalize(viewPos - fs_in.WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for(int i=0;i<4;i++){
        // 光源方向向量
        vec3 L = normalize(lightPos[i] - fs_in.WorldPos);
        vec3 H = normalize(V + L);

        float distance = length(lightPos[i] - fs_in.WorldPos);
        float attenuation = 1.0 / (distance * distance);

        // 辐照度: 反正最后要拿这个去乘
        vec3 radiance = lightColors[i] * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness); 
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	

        float NdotL = max(dot(N, L), 0.0);    
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        // Lo += radiance;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}