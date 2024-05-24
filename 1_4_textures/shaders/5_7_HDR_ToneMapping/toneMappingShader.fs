#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrMap;

uniform float exposure;
//unifrom bool hdrEnabled;

void main()
{
    //FragColor = vec4(texture(hdrMap, TexCoords).rgb, 1.0);
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrMap, TexCoords).rgb;

    vec3 result = hdrColor / (hdrColor + vec3(1.0));
    //vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / gamma));

    //FragColor = vec4(hdrColor, 1.0);
    FragColor = vec4(result, 1.0);
}