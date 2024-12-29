#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out VS_OUT {
    vec3 WorldPos;
    vec3 WorldNormal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat3 normalMatrix; // 等价于：glm::transpose(glm::inverse(glm::mat3(modelMatrix)))，在cpu中提前计算好而免去在着色器中计算

void main()
{
    vs_out.WorldPos = vec3(model * vec4(aPos, 1.0));
    vs_out.WorldNormal = normalMatrix * aNormal;
    vs_out.TexCoords = aTex;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}