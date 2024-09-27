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

    //Shader solidShader("./shaders/OIT_Weighted_Blended/solid.vs", "./shaders/OIT_Weighted_Blended/solid.fs");
    //Shader transparentShader("./shaders/OIT_Weighted_Blended/transparent.vs", "./shaders/OIT_Weighted_Blended/transparent.fs");

    Shader instancedGlassShader("./shaders/OIT_Weighted_Blended_Instancing/instancingGlassShader.vs", "./shaders/OIT_Weighted_Blended_Instancing/instancingGlassShader.fs");

    Shader compositeShader("./shaders/OIT_Weighted_Blended/composite.vs", "./shaders/OIT_Weighted_Blended/composite.fs");
    Shader screenShader("./shaders/OIT_Weighted_Blended/screen.vs", "./shaders/OIT_Weighted_Blended/screen.fs");

    myRenderEngine.SetCompositeShader(&compositeShader);
    myRenderEngine.SetScreenShader(&screenShader);

    const unsigned int amount = 999;
    auto glasses = std::make_shared<MyRenderEngine::InstancedGlass>(amount, &instancedGlassShader);

    myRenderEngine.AddRenderable(glasses);

    myRenderEngine.StartRenderLoop();

    return 0;
}