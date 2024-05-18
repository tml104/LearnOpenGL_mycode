#version 330
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    // no longer needs FragPosLightSpace
}fs_in;

uniform samplerCube depthMap;
uniform sampler2D floorTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float ShadowCalculation(vec3 normal, vec3 unnormalizeLightDir)
{
    vec3 lightDir = normalize(unnormalizeLightDir);
    // float closestDepth = texture(depthMap, lightDir).r;
    // closestDepth *= far_plane;
    float currentDepth = length(unnormalizeLightDir);

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //float shadow = currentDepth - bias > closestDepth ? 1.0: 0.0;
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fs_in.FragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i=0;i<samples;i++){
        float closestDepth = texture(depthMap, unnormalizeLightDir + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    // [debug] out put depth
    //FragColor = vec4(vec3(closestDepth / far_plane), 1.0);
    return shadow;
}

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
    
    // shadow test
    float shadow = ShadowCalculation(normal, fragPos - lightPos);
    if(!shadows) shadow = 0.0;

    return ambient + (1.0 - shadow)*(diffuse + specular);
}

void main()
{
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
    vec3 res = BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPos, viewPos) * color; // don't forget to normalize normal!
    FragColor = vec4(res, 1.0);
}