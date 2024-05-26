#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrMap;
uniform sampler2D bloomMap;

uniform float exposure;
uniform bool showMultiWindowsEnabled;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = vec3(0.0);
    // 分段输出
    if(showMultiWindowsEnabled && gl_FragCoord.y<400 && gl_FragCoord.x<800) // 原始场景
    {
        vec2 newTexCoords = vec2(gl_FragCoord.x/800.0,gl_FragCoord.y/400.0);
        hdrColor = texture(hdrMap, newTexCoords).rgb;
    }
    else if(showMultiWindowsEnabled && gl_FragCoord.y<400 && gl_FragCoord.x<800+800) // bloom
    {
        vec2 newTexCoords = vec2((gl_FragCoord.x-800.0)/800.0,gl_FragCoord.y/400.0);
        hdrColor = texture(bloomMap, newTexCoords).rgb;
    }
    else // 组合
    {
        hdrColor = texture(hdrMap, TexCoords).rgb;
        vec3 bloomColor = texture(bloomMap, TexCoords).rgb;
        hdrColor += bloomColor;
    }


    //vec3 result = hdrColor / (hdrColor + vec3(1.0));
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}