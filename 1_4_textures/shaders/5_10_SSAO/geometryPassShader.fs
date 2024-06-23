#version 330 core

// 注意：现在gPosition、gNormal在观察空间中
layout (location = 0) out vec3 gPosition;   //注意类型
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec; //注意类型

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
}fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform bool invertedNormals; // 如果是true的话表明现在在渲染的目标是物体内部（也就是箱子），全弄成白的

void main()
{
    gPosition = fs_in.FragPos;
    gNormal = normalize(fs_in.Normal);
    gAlbedoSpec.rgb = invertedNormals ? vec3(0.95): texture(texture_diffuse1, fs_in.TexCoords).rgb;
    gAlbedoSpec.a = invertedNormals ? 0.0 :texture(texture_specular1, fs_in.TexCoords).r;
}