#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D texNoise;

uniform vec3 samples[64]; // 切线空间中，需要先变换到view空间

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noiseScale = vec2(1920.0/4.0, 1080/4.0);

uniform mat4 projection;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
    // 利用贴图环绕的性质循环地读取旋转值
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal*dot(randomVec , normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i=0;i<kernelSize;i++)
    {
        vec3 samplePos = TBN * samples[i];
        // 变换到观察空间（含有位移）
        samplePos = fragPos + samplePos * radius;

        // 变换到投影空间中并手动做透视除法，然后比较深度
        vec4 offset = projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5; // to 0~1，这样才能采样

        float sampleDepth = texture(gPosition, offset.xy).z; // 注意：这里只是用offset采样，实际上比较的还是在view空间中的z值（这个值是负数因此越大越靠近相机）

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0:0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / kernelSize);

    FragColor = occlusion;
    //FragColor = pow(occlusion, 4);
}
