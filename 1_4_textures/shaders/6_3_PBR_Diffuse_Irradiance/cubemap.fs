#version 420 core
out vec4 FragColor;
in vec3 WorldPos;

layout (binding = 0) uniform sampler2D equirectangularMap; // 输入：环境立方体贴图，但是是以等距柱状投影方式存储，在读取前需要转换为球形投影

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}