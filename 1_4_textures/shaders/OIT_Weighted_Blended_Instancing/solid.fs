#version 420 core

layout (location = 0) out vec4 frag;

uniform vec4 color;

void main()
{
    frag = vec4(color.rgb, 1.0f);
}