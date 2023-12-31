﻿#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_s.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("4.1.shader.vs", "4.1.shader_2.fs"); // you can name your shader files however you like


    // new for drawing triangle
    // ---------------------------------------
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   2.0f, 2.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   2.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 2.0f  // top left 
    };

    unsigned int indices[] = {
        0,1,3,
        1,2,3
    };

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // 1. Bind VAO
    glBindVertexArray(VAO);
    // 2. Set vertices data to VBO

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // To draw a triangle, discomment next line and comment EBO
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 2.5. Set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 3. Set attribute pointer （设置如何解释顶点列表中的数据（属性））
    // 参数：属性id，属性数据个数，数据类型，是否标准化，步长（连续顶点属性组间隔，等于每个顶点属性大小）（单位为字节），
    //      该属性起始位置偏移量
    // 
    // （1）顶点坐标
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //after this is called, data in VBO is registered as the vertex attribute's bound vertex buffer object （简单来说就是不能在这个函数被调用之前解绑VBO）
    glEnableVertexAttribArray(0);

    // （2）颜色
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // （3）纹理坐标（uv）
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 4. Unbind
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 绑定完了顶点属性以后，绑定纹理属性
    // 
    // 设置反转图像
     stbi_set_flip_vertically_on_load(true);
    // 
    // 箱子
    // 1.绑定纹理对象
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 2. 设置（当前绑定纹理对象的）环绕（横纵，ST） 与 过滤（缩小与放大） 方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width_2, height_2, nrChannels_2;
    unsigned char* texture_data = stbi_load("container.jpg", &width_2, &height_2, &nrChannels_2, 0);

    if (texture_data) {

        // 参数：纹理目标, mip级别， 存储格式， 宽， 高， 总是0， 原图格式， 原图数据类型， 真正的数据来源
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_2, height_2, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
        glGenerateMipmap(GL_TEXTURE_2D);

    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(texture_data);

    // 笑脸
    // 1.绑定纹理对象
    unsigned int texture_2;
    glGenTextures(1, &texture_2);
    glBindTexture(GL_TEXTURE_2D, texture_2);

    // 2. 设置（当前绑定纹理对象的）环绕（横纵，ST） 与 过滤（缩小与放大） 方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* texture_data_2 = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);

    if (texture_data_2) {

        // 参数：纹理目标, mip级别， 存储格式， 宽， 高， 总是0， 原图格式， 原图数据类型， 真正的数据来源
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data_2);
        glGenerateMipmap(GL_TEXTURE_2D);

    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(texture_data_2);

    // sampler -> texture unit 绑定纹理单元（让sampler2D类型可以使用多种不同变量读取纹理）
    //  纹理单元：可以通过glUniform1i设置一个int数值到片段着色器的sampler2D变量中，让opengl知道使用哪个纹理单元
    ourShader.use();
    ourShader.setInt("texture1", 0); // 注意变量名与 片段着色器 中一致
    ourShader.setInt("texture2", 1);

    //线框模式
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        // change uniform for fragment shader (*new part)
        // ------
        //float timeValue = glfwGetTime();
        //float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //int vertexColorLoccation = glGetUniformLocation(shaderProgram, "ourColor");
        //glUseProgram(shaderProgram);
        //glUniform4f(vertexColorLoccation, 0.0f, greenValue, 0.0f, 1.0f);

        // bind Texture
        // 绑定纹理的时候，先激活对应纹理单元，再绑定
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_2);

        //draw triangle
        ourShader.use();
        

        //float timeValue = glfwGetTime();
        //int displacementLoccation = glGetUniformLocation(ourShader.ID, "ourDisplacement");
        //glUseProgram(ourShader.ID);
        //glUniform3f(displacementLoccation, 0.0, sin(timeValue), 0.0);


        glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0); //no need to bind every time


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // delete
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}