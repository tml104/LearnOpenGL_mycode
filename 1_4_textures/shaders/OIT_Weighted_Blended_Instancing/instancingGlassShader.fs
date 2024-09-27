#version 420 core

layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;
layout (binding = 0) uniform sampler2D textureTransparent;

in vec2 TexCoords;

void main()
{

    vec4 color = texture(textureTransparent, TexCoords);

    float weight = clamp(
        pow(min(1.0, color.a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0),
        1e-2,
        3e3
    ); // 这是一个和片段深度有关系的函数

    accum = vec4(color.rgb * color.a, color.a) * weight;

    reveal = color.a;
}