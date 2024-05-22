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
uniform sampler2D heightMap;

uniform float height_scale;
uniform bool parallaxMappingEnabled;

// vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
// {
//     float height = texture(heightMap, texCoords).r;
//     vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
//     return texCoords - p;
// }

vec2 SteepParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // number of depth layers
    // perpendicular -> minLayers, parallel -> maxLayers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(heightMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    //return currentTexCoords; // x2
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords; // x1
    float afterDepth = currentDepthMapValue - currentLayerDepth; // z2 - y2
    float beforeDepth = texture(heightMap, prevTexCoords).r - (currentLayerDepth - layerDepth); // z1 - y1

    float t = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * t + currentTexCoords * (1.0 - t); // x1 * t + x2 * (1-t)

    return finalTexCoords;
}

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 viewPos)
{
    vec3 lightColor = vec3(0.5);
    //ambient
    vec3 ambient = 0.1 * lightColor;

    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;    

    // simple attenuation
    // float max_distance = 1.5; //no use??
    // float distance = length(lightPos - fragPos);
    // float attenuation = 1.0 / (distance);
    
    // ambient *= attenuation;
    // diffuse *= attenuation;
    // specular *= attenuation;
    

    return ambient + diffuse + specular;
}

void main()
{           
    // Obtain normal from normal map in range [0,1]
    vec3 normal;
    // Transform normal vector to range [-1,1]
    

    vec3 color;
    if(parallaxMappingEnabled)
    {
        vec2 texCoordsParallaxed = SteepParallaxMapping(fs_in.TexCoords, normalize(fs_in.TangentViewPos - fs_in.TangentFragPos)); // 注意：这里是normalize过的

        // if(
        //     texCoordsParallaxed.x>1.0 ||
        //     texCoordsParallaxed.y>1.0 ||
        //     texCoordsParallaxed.x<0.0 || 
        //     texCoordsParallaxed.y<0.0
        // ){
        //     discard;
        // }
        normal = texture(normalMap, texCoordsParallaxed).rgb;
        color = texture(diffuseMap, texCoordsParallaxed).rgb;
    }
    else{
        normal = texture(normalMap, fs_in.TexCoords).rgb;
        color = texture(diffuseMap, fs_in.TexCoords).rgb;
    }

    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    vec3 res = BlinnPhong(normal, fs_in.TangentFragPos, fs_in.TangentLightPos, fs_in.TangentViewPos) * color; // don't forget to normalize normal!


    FragColor = vec4(res, 1.0);

}