#version 420 core
out vec3 WorldPos;

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

void main(){
    WorldPos = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}