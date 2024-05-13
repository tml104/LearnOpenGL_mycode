#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    // 这个地方先用model矩阵变换到世界坐标，然后立即使用文中描述的T矩阵（也即lightSpaceMatrix，其是）
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}