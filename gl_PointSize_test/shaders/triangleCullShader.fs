#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;



void main()
{    
    if(gl_FrontFacing)
    {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0); // blue
    }else{

        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // white
    }

}

