#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;

float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);


void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 res = texture(image, TexCoords).rgb * weight[0];

    if(horizontal)
    {
        for(int i=1;i<5;i++)
        {
            // note: Pay attention to the way the texture wraps
            vec2 displacement = vec2(tex_offset.x * i, 0.0);
            res += texture(image, TexCoords + displacement).rgb * weight[i];
            res += texture(image, TexCoords - displacement).rgb * weight[i];
        }
    }
    else
    {
        for(int i=1;i<5;i++)
        {
            // note: Pay attention to the way the texture wraps
            vec2 displacement = vec2(0.0, tex_offset.y * i);
            res += texture(image, TexCoords + displacement).rgb * weight[i];
            res += texture(image, TexCoords - displacement).rgb * weight[i];
        }
    }

    FragColor = vec4(res, 1.0);
}
