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

namespace MyRenderEngine {

	// Configs
	const glm::vec4 GREEN_BACKGROUND{ 0.2f, 0.3f, 0.3f, 1.0f };
	const glm::vec4 BLACK_BACKGROUND{ 0.0f, 0.0f, 0.0f, 0.0f };

	const glm::vec3 CAMERA_INIT_POS{ 0.0f, 0.0f, 5.0f };

	const unsigned int SCR_WIDTH = 1024;
	const unsigned int SCR_HEIGHT = 768;
	const unsigned int SCR_X_POS = 200;
	const unsigned int SCR_Y_POS = 200;

	const glm::vec4 ZERO_VEC(0.0f);
	const glm::vec4 ONE_VEC(1.0f);

	/*
		由MyRenderEngine传入IRenderable中
	*/
	struct RenderInfo {
		glm::mat4 projection_matrix;
		glm::mat4 view_matrix;
		glm::vec3 camera_pos;
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

		std::vector<std::shared_ptr<IRenderable>> opaqueRenderables, transparentRenderables; // 渲染对象列表

		// Framebuffer
		// TODO: 生命周期闭环
		unsigned int opaqueFBO;
		unsigned int transparentFBO;

		unsigned int opaqueTexture;
		unsigned int depthTexture;

		unsigned int accumTexture;
		unsigned int revealTexture;

		// screenQuad
		unsigned int screenQuadVAO;
		unsigned int screenQuadVBO;
		int screenQuadVerticesCount;

		Shader* compositeShader; // OIT Use
		Shader* screenShader;

		// glfw: whenever the window size changed (by OS or user resize) this callback function executes
		void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
			// make sure the viewport matches the new window dimensions; note that width and 
			// height will be significantly larger than specified on retina displays.
			screenWidth = width;
			screenHeight = height;
			glViewport(0, 0, width, height);
		}

		void AddRenderable(const std::shared_ptr<IRenderable>& r) {
			if (r->GetRenderableInfo().isOpaque) {
				opaqueRenderables.emplace_back(r);
			}
			else {
				transparentRenderables.emplace_back(r);
			}
		}

		void SetCameraPos(const glm::vec3& pos) {
			camera.Position = pos;
		}

		void SetCompositeShader(Shader* shader) {
			compositeShader = shader;
		}

		void SetScreenShader(Shader* shader){
			screenShader = shader;
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

		void SetupScreenQuad() {
			static float quadVertices[] = {
				// positions        // uv
				-1.0f, -1.0f, 0.0f,	0.0f, 0.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,

				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
			};

			screenQuadVerticesCount = 6;

			glGenVertexArrays(1, &screenQuadVAO);
			glGenBuffers(1, &screenQuadVBO);

			glBindVertexArray(screenQuadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glBindVertexArray(0);
		}

		void SetupGlobalOpenglState() {
			glEnable(GL_DEPTH_TEST);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glDepthFunc(GL_LESS); // 深度缓冲比较通过条件：小于 （默认就是这个吧）
			glDepthMask(GL_TRUE); // 允许更新深度缓冲
			glDisable(GL_BLEND);
			glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
		}

		void SetupFrameBuffers() {
			glGenFramebuffers(1, &opaqueFBO);
			glGenFramebuffers(1, &transparentFBO);

			// opaqueTexture
			glGenTextures(1, &opaqueTexture);
			glBindTexture(GL_TEXTURE_2D, opaqueTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_HALF_FLOAT, NULL); // 注意这里用的是 gl_half_float
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			// depthTexture
			glGenTextures(1, &depthTexture);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Bind opaqueTexture & depthTexture to opaqueFBO
			glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, opaqueTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Opaque framebuffer is not complete!" << std::endl;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// set up attachments for transparent framebuffer
			// accumTexture
			glGenTextures(1, &accumTexture);
			glBindTexture(GL_TEXTURE_2D, accumTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			// revealTexture
			glGenTextures(1, &revealTexture);
			glBindTexture(GL_TEXTURE_2D, revealTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL); // 注意这里因为只有一个通道所以会有所不同！
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Bind accumTexture & revealTexture & depthTexture to transparent
			glBindFramebuffer(GL_FRAMEBUFFER, transparentFBO);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, revealTexture, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0); // opaque framebuffer's depth texture

			// don't forget to explicitly tell OpenGL that your transparent framebuffer has two draw buffers
			const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, transparentDrawBuffers);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Transparent framebuffer is not complete!" << std::endl;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}


		void StartRenderLoop() {

			RenderInfo renderInfo;

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
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
				glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);

				glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				for (auto&& r : opaqueRenderables) {
					r->Render(renderInfo);
				}

				// -> Transparent (transparent pass)
				glDepthMask(GL_FALSE);
				glEnable(GL_BLEND);
				glBlendFunci(0, GL_ONE, GL_ONE);
				glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glBlendEquation(GL_FUNC_ADD);

				glBindFramebuffer(GL_FRAMEBUFFER, transparentFBO);
				glClearBufferfv(GL_COLOR, 0, &ZERO_VEC[0]); // 新函数
				glClearBufferfv(GL_COLOR, 1, &ONE_VEC[0]);

				for (auto&& r : transparentRenderables) {
					r->Render(renderInfo);
				}

				// render composite image (composite pass)
				glDepthFunc(GL_ALWAYS);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBindFramebuffer(GL_FRAMEBUFFER, opaqueFBO);

				compositeShader->use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, accumTexture);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, revealTexture);

				glBindVertexArray(screenQuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, screenQuadVerticesCount);

				// render screenQuad (draw to backbuffer) (final pass)
				
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE); // enable depth writes so glClear won't ignore clearing the depth buffer
				glDisable(GL_BLEND);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				screenShader->use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, opaqueTexture);
				glBindVertexArray(screenQuadVAO);
				glDrawArrays(GL_TRIANGLES, 0, screenQuadVerticesCount);


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
			SetupScreenQuad();
			//SetupGlobalOpenglState();
			SetupFrameBuffers();
		}

		~MyRenderEngine() {
			glDeleteVertexArrays(1, &screenQuadVAO);
			glDeleteBuffers(1, &screenQuadVBO);
			glDeleteTextures(1, &opaqueTexture);
			glDeleteTextures(1, &depthTexture);
			glDeleteTextures(1, &accumTexture);
			glDeleteTextures(1, &revealTexture);
			glDeleteFramebuffers(1, &opaqueFBO);
			glDeleteFramebuffers(1, &transparentFBO);

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
			//glfwWindowHint(GLFW_SAMPLES, 4);

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

} // namespace MyRenderEngine