#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/gl.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

// Camera
ew::CameraController cameraController;
ew::Camera camera;
void resetCamera();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;
glm::vec3 ambient_color;

struct Material
{
	float ka = 0.5;
	float kd = 0.5;
	float ks = 0.5;
	float shininess = 128;
}material;

struct GBuffer
{
	unsigned int fbo;

	unsigned int position;
	unsigned int normal;
	unsigned int albedo;
	unsigned int depth;
}deffered;

void CreateDeferredPass()
{
	// Create FBO
	glGenFramebuffers(1, &deffered.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, deffered.fbo);

	// Create Position Buffer
	glGenTextures(1, &deffered.position);
	glBindTexture(GL_TEXTURE_2D, deffered.position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Bind Position Buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deffered.position, 0);

	// Create Normal Buffer
	glGenTextures(1, &deffered.normal);
	glBindTexture(GL_TEXTURE_2D, deffered.normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Bind Normal Buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deffered.normal, 0);

	// Create Albedo Buffer
	glGenTextures(1, &deffered.albedo);
	glBindTexture(GL_TEXTURE_2D, deffered.albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Bind Albedo Buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deffered.albedo, 0);

	// Create Depth Map
	glGenTextures(1, &deffered.depth);
	glBindTexture(GL_TEXTURE_2D, deffered.depth);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	const float border_color[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	// Bind Depth Buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, deffered.depth, 0);

	const unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// Check FBO
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(0 && "Frame Buffer Not Complete");

	// Unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);

	//ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	//ew::Shader defferedShader = ew::Shader("assets/deffered.vert", "assets/deffered.frag");
	ew::Shader gPass = ew::Shader("assets/gPass.vert", "assets/gPass.frag");

	ew::Model model = ew::Model("assets/suzanne.obj");
	ew::Transform modelTransform;

	GLuint texture = ew::loadTexture("assets/texture.jpg");

	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = static_cast<float>(screenWidth) / screenHeight;
	camera.fov = 60;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	CreateDeferredPass();

	// Render Settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float time = static_cast<float>(glfwGetTime());
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));
		cameraController.move(window, &camera, deltaTime);

		// Geometry Pass
		{
			glBindFramebuffer(GL_FRAMEBUFFER, deffered.fbo);

			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Bind textures
			glBindTextureUnit(0, texture);

			// Set uniforms
			gPass.use();
			gPass.setInt("main_texture", 0);

			gPass.setMat4("model_matrix", modelTransform.modelMatrix());
			gPass.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());
			model.draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Lighting Pass
		{
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		// Render UI
		{
			drawUI();
		}

		glfwSwapBuffers(window);
	}

	printf("Shutting down...");
}

void resetCamera()
{
	camera.position = glm::vec3(0, 0, 5.0f);
	camera.target = glm::vec3(0);
	cameraController.yaw = cameraController.pitch = 0;
}

void drawUI()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera"))
	{
		resetCamera();
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::SliderFloat("AmbientK", &material.ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 1024.0f);
	}

	ImGui::End();

	{
		ImGui::Begin("Position");
		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)deffered.position, WindowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	{
		ImGui::Begin("Normal");
		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)deffered.normal, WindowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	{
		ImGui::Begin("Color");
		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)deffered.albedo, WindowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	{
		ImGui::Begin("Depth");
		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImGui::Image((ImTextureID)deffered.depth, WindowSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
/// 
GLFWwindow* initWindow(const char* title, int width, int height)
{
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
	{
		printf("GLFW failed to create window");
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress))
	{
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}