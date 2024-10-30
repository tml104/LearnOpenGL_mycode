#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_s.h"
#include "camera.h"

#include "model.h"
#include "mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <random>
#include <memory>

#include "MyRenderEngine.hpp"

int main()
{
    MyRenderEngine::MyRenderEngine myRenderEngine;

    Shader pbrShader("./shaders/6_2_PBR/pbr.vs", "./shaders/6_2_PBR/pbr.fs");

    //std::shared_ptr<MyRenderEngine::Quad> redQuad = std::make_shared<MyRenderEngine::Quad>(1, &(solidShader));
    //std::shared_ptr<MyRenderEngine::Quad> greenQuad = std::make_shared<MyRenderEngine::Quad>(2, &(transparentShader));
    //std::shared_ptr<MyRenderEngine::Quad> blueQuad = std::make_shared<MyRenderEngine::Quad>(3, &(transparentShader));

    const int SPHERE_ROW_SIZE = 10;
    const int SPHERE_COL_SIZE = 10;

    for (int i = 0; i < SPHERE_ROW_SIZE; i++) {
        for (int j = 0; j < SPHERE_COL_SIZE; j++) {

            auto sphere = std::make_shared<MyRenderEngine::Sphere>(i,j, SPHERE_ROW_SIZE, SPHERE_COL_SIZE,&pbrShader);
            myRenderEngine.AddRenderable(sphere);
        }
    }

    // Add lights
    auto l1 = std::make_shared<MyRenderEngine::PointLight>(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f));
    auto l2 = std::make_shared<MyRenderEngine::PointLight>(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f));
    auto l3 = std::make_shared<MyRenderEngine::PointLight>(glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f));
    auto l4 = std::make_shared<MyRenderEngine::PointLight>(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f));

    myRenderEngine.AddLight(l1);
    myRenderEngine.AddLight(l2);
    myRenderEngine.AddLight(l3);
    myRenderEngine.AddLight(l4);

    myRenderEngine.StartRenderLoop();
    return 0;
}

// utility function for loading a 2D texture from file
// 绑定纹理对象与实际数据，并返回纹理对象ID以供随后激活纹理单元并将纹理对象与纹理单元绑定
// ---------------------------------------------------
unsigned int loadTexture(char const* path, bool gammaCorrection) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data) {
        GLenum dataFormat;
        GLenum internalFormat;
        if (nrComponents == 1) {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4) {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        // 绑定：绑定纹理对象与实际数据
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 纹理环绕方式（与绑定之间的顺序随意）
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

    }
    else {
        std::cout << "Texture failed to load at path:" << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}