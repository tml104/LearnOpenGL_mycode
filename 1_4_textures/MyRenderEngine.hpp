#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <cmath>

#include "shader_s.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 


namespace MyRenderEngine {

	// Configs
	const glm::vec4 GREEN_BACKGROUND{ 0.2f, 0.3f, 0.3f, 1.0f };
	const glm::vec4 BLACK_BACKGROUND{ 0.0f, 0.0f, 0.0f, 0.0f };

	const glm::vec3 CAMERA_INIT_POS{ 0.0f, 0.0f, 5.0f };

	const unsigned int SCR_WIDTH = 1920;
	const unsigned int SCR_HEIGHT = 1080;
	const unsigned int SCR_X_POS = 200;
	const unsigned int SCR_Y_POS = 200;

	const glm::vec4 ZERO_VEC(0.0f);
	const glm::vec4 ONE_VEC(1.0f);

	class ILight;
	/*
		由MyRenderEngine传入IRenderable中
	*/
	struct RenderInfo {
		glm::mat4 projection_matrix;
		glm::mat4 view_matrix;
		glm::vec3 camera_pos;

		std::vector<std::shared_ptr<ILight>> lights; //有待改进，因为这样其实位置数据不是组织在一起的
	};

	/*
		由IRenderable传入MyRenderEngine中
	*/
	struct RenderableInfo {
		int isOpaque;
	};

	class IRenderable {
	public:
		virtual void Render(
			const RenderInfo& renderInfo
		) = 0;
		virtual ~IRenderable() {};

		virtual glm::mat4 GetModelMatrix() = 0;
		virtual RenderableInfo GetRenderableInfo() = 0;
	};

	class ILight {
	public:
		virtual glm::vec3 GetPos() const = 0;
		virtual glm::vec3 GetLightColor() const = 0;

		virtual ~ILight() {}
	};

	class PointLight: public ILight {
	public:
		glm::vec3 pos;
		glm::vec3 color;
		PointLight(glm::vec3 pos, glm::vec3 color): pos(pos), color(color) {}

		glm::vec3 GetPos() const override {
			return pos;
		}
		glm::vec3 GetLightColor() const override {
			return color;
		}
	};

	glm::mat4 CalculateModelMatrix(const glm::vec3& position, const glm::vec3& rotation = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f)) { // 原来引用能设置默认变量吗……
		glm::mat4 trans = glm::mat4(1.0f);

		trans = glm::translate(trans, position);
		trans = glm::rotate(trans, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
		trans = glm::rotate(trans, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
		trans = glm::rotate(trans, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
		trans = glm::scale(trans, scale);

		return trans;
	}

	/*
		使用两个状态维护切换状态
	*/
	void ProcessToggleKey(GLFWwindow* window, int key, bool& var, bool& varPressed) {
		if (glfwGetKey(window, key) == GLFW_PRESS && !varPressed) {
			var = !var;
			varPressed = true;

			std::cout << key << ": " << var << std::endl;
		}
		if (glfwGetKey(window, key) == GLFW_RELEASE) {
			varPressed = false;
		}
	}

	class MouseController {

	public:
		void MouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
			float xpos = static_cast<float>(xposIn);
			float ypos = static_cast<float>(yposIn);

			if (firstMouse) {
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = -ypos + lastY; // reversed since y-coordinates go from bottom to top
			lastX = xpos;
			lastY = ypos;

			camera.ProcessMouseMovement(xoffset, yoffset);
		}

		void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
			camera.ProcessMouseScroll(static_cast<float>(yoffset));
		}

		MouseController(Camera& icamera) :
			lastX(0.0f),
			lastY(0.0f),
			firstMouse(true),
			camera(icamera)
		{}

	private:
		Camera& camera;
		bool firstMouse;
		float lastX;
		float lastY;
	};

	class KeyboardController {
	public:
		float keyboardMovementSpeed;

		void KeyboardProcessInput(GLFWwindow* window, float deltaTime) {
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);

			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
				camera.ChangeMovementSpeed(0.1);
				std::cout << "MovementSpeed: " << camera.MovementSpeed << std::endl;
			}

			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
				camera.ChangeMovementSpeed(-0.1);
				std::cout << "MovementSpeed: " << camera.MovementSpeed << std::endl;
			}

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				camera.ProcessKeyboard(FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				camera.ProcessKeyboard(BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				camera.ProcessKeyboard(LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				camera.ProcessKeyboard(RIGHT, deltaTime);

			// 为了方便远程的尝试
			if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
				camera.ProcessMouseMovement(0.0f, keyboardMovementSpeed);
			if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
				camera.ProcessMouseMovement(0.0f, -keyboardMovementSpeed);
			if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
				camera.ProcessMouseMovement(-keyboardMovementSpeed, 0.0f);
			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				camera.ProcessMouseMovement(keyboardMovementSpeed, 0.0f);

		}

		KeyboardController(Camera& icamera) :
			camera(icamera),
			keyboardMovementSpeed(5.0f)
		{}

	private:
		Camera& camera;
	};

	/*
		注意：必须要最先初始化这个东西（因为glfw必须先做初始化），之后设置着色器之类的才能成功执行
	*/
	class MyRenderEngine {

	public:
		// settings
		unsigned int screenWidth;
		unsigned int screenHeight;
		unsigned int screenXPos;
		unsigned int screenYPos;
		glm::vec4 backgroundColor;

		GLFWwindow* window;
		Camera camera;
		MouseController mouseController;
		KeyboardController keyboardController;

		std::vector<std::shared_ptr<IRenderable>> opaqueRenderables; // 渲染对象列表
		std::vector<std::shared_ptr<ILight>> lights; // 光源对象

		// 标记为 [后加] 者，需要在调用StartRenderLoop前手动赋值
		// （懒得写一堆函数了，直接给public成员赋值吧）

		std::shared_ptr<IRenderable> cube; // [后加] 背景立方体贴图渲染对象

		// FrameBuffer
		unsigned int captureFBO;
		unsigned int captureRBO;

		// textures
		unsigned int envCubemap;
		unsigned int hdrTexture; // [后加]

		// shaders
		Shader* equirectangularToCubemapShader;// [后加]
		Shader* backgroundShader;// [后加]

		// glfw: whenever the window size changed (by OS or user resize) this callback function executes
		void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
			// make sure the viewport matches the new window dimensions; note that width and 
			// height will be significantly larger than specified on retina displays.
			screenWidth = width;
			screenHeight = height;
			glViewport(0, 0, width, height);
		}

		void AddRenderable(const std::shared_ptr<IRenderable>& r) {
			opaqueRenderables.emplace_back(r);
		}

		void AddLight(const std::shared_ptr<ILight>& l) {
			lights.emplace_back(l);
		}

		void SetCameraPos(const glm::vec3& pos) {
			camera.Position = pos;
		}

#ifdef USE_IMGUI
		void SetupImGui() {

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsLight();

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(window, true);
			ImGui_ImplOpenGL3_Init("#version 130");
		}
#endif

		void SetupGlobalOpenglState() {
			glEnable(GL_DEPTH_TEST);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glDepthFunc(GL_LEQUAL); // 深度缓冲比较通过条件：小于等于
			glDepthMask(GL_TRUE); // 允许更新深度缓冲
			glDisable(GL_BLEND);
			glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
		}

		void SetupFrameBuffers() {
			glGenFramebuffers(1, &captureFBO);
			glGenRenderbuffers(1, &captureRBO);

			glGenTextures(1, &envCubemap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			for (unsigned int i = 0; i < 6; i++) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		void StartRenderCubemap() {
			glCheckError();
			glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

			glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			glm::mat4 captureViews[] =
			{
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
				glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
			};

			equirectangularToCubemapShader->use();
			equirectangularToCubemapShader->setMatrix4("projection", captureProjection);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, hdrTexture);

			RenderInfo renderInfo;
			glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
			for (unsigned int i = 0; i < 6; i++) {
				equirectangularToCubemapShader->setMatrix4("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				cube->Render(renderInfo);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glCheckError();
		}

		void StartRenderLoop() {

			RenderInfo renderInfo;
			renderInfo.lights = lights;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			while (!glfwWindowShouldClose(window)) {
				// poll IO events (keys pressed/released, mouse moved etc.)
				glfwPollEvents();

#ifdef USE_IMGUI
				// Imgui settings first
				if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
				{
					ImGui_ImplGlfw_Sleep(10);
					continue;
				}

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				ImGui::ShowDemoWindow();
#endif

				// per-frame time logic
				float currentFrame = static_cast<float>(glfwGetTime());
				deltaTime = currentFrame - lastFrame;
				lastFrame = currentFrame;

				// input
				// -----
				keyboardController.KeyboardProcessInput(window, deltaTime);

				// render
				// -----
				int display_w, display_h;
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);

				glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(screenWidth) / static_cast<float>(screenHeight), camera.Near, camera.Far);
				glm::mat4 viewMatrix = camera.GetViewMatrix();

				renderInfo.projection_matrix = projectionMatrix;
				renderInfo.view_matrix = viewMatrix;
				renderInfo.camera_pos = camera.Position;

				// render IRenderable to opaqueFBO & transparentFBO
				// -> Opaque (solid pass)
				//glEnable(GL_DEPTH_TEST);
				//glDepthFunc(GL_LESS);
				//glDepthMask(GL_TRUE);
				//glDisable(GL_BLEND);
				glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				for (auto&& r : opaqueRenderables) {
					r->Render(renderInfo);
				}

				// render skybox cubemap
				backgroundShader->use();
				backgroundShader->setMatrix4("projection", projectionMatrix); // 别忘了这个……
				backgroundShader->setMatrix4("view", viewMatrix);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
				cube->Render(renderInfo);


#ifdef USE_IMGUI
				// Imgui rendering
				//ImGui::Render();
				//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

				// glfw: swap buffers
				// -------------------------------------------------------------------------------
				glfwSwapBuffers(window);
			}

			// glfw: terminate, clearing all previously allocated GLFW resources.
			// ------------------------------------------------------------------
			glfwTerminate();
		}

		MyRenderEngine() :
			camera(CAMERA_INIT_POS),
			mouseController(camera),
			keyboardController(camera),
			deltaTime(0.0f),
			lastFrame(0.0f),

			screenWidth(SCR_WIDTH),
			screenHeight(SCR_HEIGHT),
			screenXPos(SCR_X_POS),
			screenYPos(SCR_Y_POS),
			backgroundColor(BLACK_BACKGROUND)
		{
			int init_res = InitWindow(window);
			if (init_res != 0) {
				throw std::runtime_error("Init window failed");
			}
			glfwMakeContextCurrent(window);
			glfwSwapInterval(1); // Enable vsync

#ifdef USE_IMGUI
			SetupImGui();
#endif
			SetupGlobalOpenglState();
			SetupFrameBuffers();
		}

		~MyRenderEngine() {
			// TODO: 释放资源
			std::cout << "MyrenderEngine Destructor executed." << std::endl;
		}

	private:
		float deltaTime;
		float lastFrame;

		int InitWindow(GLFWwindow*& window) {
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
			window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL MyCode", NULL, NULL);
			if (window == nullptr) {
				std::cout << "Failed to create GLFW window" << std::endl;
				glfwTerminate();
				return -1;
			}
			glfwMakeContextCurrent(window);
			glfwSetWindowPos(window, screenXPos, screenYPos);

			auto framebufferSizeCallback = [](GLFWwindow* window, int width, int height) {
				MyRenderEngine* app = static_cast<MyRenderEngine*>(glfwGetWindowUserPointer(window));
				if (app) {
					app->FramebufferSizeCallback(window, width, height);
				}
				};

			auto mouseCallback = [](GLFWwindow* window, double xposIn, double yposIn) {
				MyRenderEngine* app = static_cast<MyRenderEngine*>(glfwGetWindowUserPointer(window));
				if (app) {
					app->mouseController.MouseCallback(window, xposIn, yposIn);
				}
				};

			auto scrollCallback = [](GLFWwindow* window, double xoffset, double yoffset) {
				MyRenderEngine* app = static_cast<MyRenderEngine*>(glfwGetWindowUserPointer(window));
				if (app) {
					app->mouseController.ScrollCallback(window, xoffset, yoffset);
				}
				};

			glfwSetWindowUserPointer(window, this);
			glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
			glfwSetCursorPosCallback(window, mouseCallback);
			glfwSetScrollCallback(window, scrollCallback);

			// tell GLFW to capture our mouse
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			// glad: load all OpenGL function pointers
			// ---------------------------------------
			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				std::cout << "Failed to initialize GLAD" << std::endl;
				return -1;
			}

			return 0;
		}
	};


	class Quad : public IRenderable {
	public:

		unsigned int quadVAO;
		unsigned int quadVBO;

		int verticesCount;

		glm::mat4 modelMatrix;
		glm::vec4 renderColor;

		int colorFlag;
		bool isOpaque;

		Shader* shader;

		void Render(const RenderInfo& renderInfo) override {
			shader->use();

			shader->setMatrix4("projection", renderInfo.projection_matrix);
			shader->setMatrix4("view", renderInfo.view_matrix);
			shader->setMatrix4("model", modelMatrix);

			shader->setVec4("color", renderColor);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, verticesCount);
		}

		glm::mat4 GetModelMatrix() override {
			return modelMatrix;
		}

		RenderableInfo GetRenderableInfo() override {
			return { isOpaque };
		}

		Quad(int color_flag, Shader* shader) : colorFlag(color_flag), shader(shader) {
			static float quadVertices[] = {
				// positions        // uv
				-1.0f, -1.0f, 0.0f,	0.0f, 0.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,

				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
			};

			verticesCount = 6;

			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);

			glBindVertexArray(quadVAO);
				glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glBindVertexArray(0);

			// 通过颜色决定modelMatrix矩阵的位置情况
			if (color_flag == 1) { // RED （不透明）
				modelMatrix = CalculateModelMatrix(glm::vec3(0.0f, 0.0f, 1.0f));
				renderColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
				isOpaque = true;
			}
			else if (color_flag == 2) { // GREEN （透明）
				modelMatrix = CalculateModelMatrix(glm::vec3(0.0f, 0.0f, 3.0f));
				renderColor = glm::vec4(0.0, 1.0, 0.0, 0.5);
				isOpaque = false;
			}
			else if (color_flag == 3) { // BLUE （透明）
				modelMatrix = CalculateModelMatrix(glm::vec3(0.0f, 0.0f, 2.0f));
				renderColor = glm::vec4(0.0, 0.0, 1.0, 0.5);
				isOpaque = false;
			}
		}
	};

	struct PBR{
		unsigned int albedo_texture;
		unsigned int normal_texture;
		unsigned int metallic_texture;
		unsigned int roughness_texture;
		unsigned int ao_texture;
	};

	struct HDRTexture {
		unsigned int hdr_texture;
	};

	class Cube : public IRenderable {
	public:
		static unsigned int cubeVAO; 
		static unsigned int cubeVBO;

		glm::mat4 GetModelMatrix() override {
			return glm::mat4{};
		}

		RenderableInfo GetRenderableInfo() override {
			return { true };
		}

		void Render(const RenderInfo& renderInfo) override {
			// render Cube (without shader here)
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}

		void BuildVAO() {
			static float vertices[] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
				// front face
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
				-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
				-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
				// left face
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
				-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
				// right face
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
				 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
				 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
				 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
				 // bottom face
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
				 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
				 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
				 // top face
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
				  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
				 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
				 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
		
			glGenVertexArrays(1, &cubeVAO);
			glGenBuffers(1, &cubeVBO);

			// fill buffer
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			// link vertex attributes
			glBindVertexArray(cubeVAO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		Cube() {
			BuildVAO();
		}

		~Cube() override {
			// 释放资源
			glDeleteVertexArrays(1, &cubeVAO);
			glDeleteBuffers(1, &cubeVBO);
		}
	
	};

	unsigned int Cube::cubeVAO = 0;
	unsigned int Cube::cubeVBO = 0;

	class Sphere : public IRenderable {
	public:
		static unsigned int sphereVAO;
		static unsigned int sphereVBO;
		static unsigned int sphereEBO;
		static unsigned int indexCount;

		Shader* shader;
		glm::mat4 modelMatrix;
		PBR pbr;

		void Render(const RenderInfo& renderInfo) override {
			shader->use();

			shader->setMatrix4("projection", renderInfo.projection_matrix);
			shader->setMatrix4("view", renderInfo.view_matrix);
			shader->setMatrix4("model", modelMatrix);

			shader->setMatrix3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix)))); // 注意这个的类型

			// 相机位置
			shader->setVec3("viewPos", renderInfo.camera_pos);

			// 光源设置
			for (int i = 0; i < renderInfo.lights.size(); i++) {
				auto l = renderInfo.lights[i];
				shader->setVec3("lightPos[" + std::to_string(i) + "]", l->GetPos());
				shader->setVec3("lightColors[" + std::to_string(i) + "]", l->GetLightColor());
			}

			// 材质设置
			//shader->setVec3("albedo", pbr.albedo);
			//shader->setFloat("metallic", pbr.metallic);
			//shader->setFloat("roughness", pbr.roughness);
			//shader->setFloat("ao", pbr.ao);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pbr.albedo_texture);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pbr.normal_texture);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pbr.metallic_texture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pbr.roughness_texture);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pbr.ao_texture);

			glBindVertexArray(sphereVAO);
			glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
		}

		glm::mat4 GetModelMatrix() override {
			return modelMatrix;
		}

		RenderableInfo GetRenderableInfo() override {
			return { true };
		}

		// 建立第一个sphereVAO
		void BuildVAO() {
			// 建立第一个sphereVAO
			glGenVertexArrays(1, &sphereVAO);

			glGenBuffers(1, &sphereVBO);
			glGenBuffers(1, &sphereEBO);

			std::vector<glm::vec3> positions;
			std::vector<glm::vec2> uv;
			std::vector<glm::vec3> normals;
			std::vector<unsigned int> indices;

			const unsigned int X_SEGMENTS = 64;
			const unsigned int Y_SEGMENTS = 64;
			const float PI = 3.14159265359f;
			// 球坐标映射
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
				{
					float xSegment = (float)x / (float)X_SEGMENTS;
					float ySegment = (float)y / (float)Y_SEGMENTS;
					float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
					float yPos = std::cos(ySegment * PI);
					float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

					positions.push_back(glm::vec3(xPos, yPos, zPos));
					uv.push_back(glm::vec2(xSegment, ySegment));
					normals.push_back(glm::vec3(xPos, yPos, zPos));
				}
			}

			bool oddRow = false;
			for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
			{
				if (!oddRow) // even rows: y == 0, y == 2; and so on
				{
					for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
					{
						indices.push_back(y * (X_SEGMENTS + 1) + x);
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					}
				}
				else
				{
					for (int x = X_SEGMENTS; x >= 0; --x)
					{
						indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
						indices.push_back(y * (X_SEGMENTS + 1) + x);
					}
				}
				oddRow = !oddRow;
			}
			indexCount = static_cast<unsigned int>(indices.size());

			std::vector<float> data;
			for (unsigned int i = 0; i < positions.size(); ++i)
			{
				data.push_back(positions[i].x);
				data.push_back(positions[i].y);
				data.push_back(positions[i].z);
				if (normals.size() > 0)
				{
					data.push_back(normals[i].x);
					data.push_back(normals[i].y);
					data.push_back(normals[i].z);
				}
				if (uv.size() > 0)
				{
					data.push_back(uv[i].x);
					data.push_back(uv[i].y);
				}
			}

			glBindVertexArray(sphereVAO);
			glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			unsigned int stride = (3 + 2 + 3) * sizeof(float);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
			glBindVertexArray(0);
		}

		Sphere(int row, int col, int row_size, int col_size, PBR pbr, Shader* shader) : shader(shader), pbr(pbr) {
			if (sphereVAO == 0) {
				BuildVAO();
			}

			// modelMatrix, pbr
			static const float SPACEING = 2.5f;
			glm::vec3 translate = glm::vec3(row * SPACEING, col * SPACEING, 0.0f);
			modelMatrix = CalculateModelMatrix(translate);

			//pbr.albedo = glm::vec3(0.5f, 0.0f, 0.0f);
			//pbr.ao = 1.0f;
			//pbr.metallic = row * 1.0f / row_size;
			//pbr.roughness = glm::clamp(col * 1.0f / col_size, 0.05f, 1.0f);
		}

		~Sphere() override {
			// 释放资源
			glDeleteVertexArrays(1, &sphereVAO);
			glDeleteBuffers(1, &sphereVBO);
			glDeleteBuffers(1, &sphereEBO);
		}

	};

	unsigned int Sphere::sphereVAO = 0;
	unsigned int Sphere::sphereVBO = 0;
	unsigned int Sphere::sphereEBO = 0;
	unsigned int Sphere::indexCount = 0;

} // namespace MyRenderEngine