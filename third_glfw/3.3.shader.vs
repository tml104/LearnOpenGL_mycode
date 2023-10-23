#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourPos;
void main()
{
   // gl_Position = vec4(aPos.xyz, 1.0);

   // upside down

   // vec3 aPos_res = ourDisplacement + aPos;
   // gl_Position = vec4(aPos.x + ourDisplacement.x, aPos.y - ourDisplacement.y, aPos.z+ourDisplacement.z, 1.0);


   gl_Position = vec4(aPos, 1.0);
   ourPos = aPos;
}