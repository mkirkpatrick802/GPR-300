#include <stdio.h>
#include <math.h>

// EW
#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

// LIBS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// OUR FILES
#include "Framebuffer.h"
#include "CrosshatchingShader.h"
#include "OutlineShader.h"
#include "NoiseShader.h"

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
	float ka = 1.0;
	float kd = 0.5;
	float ks = 0.5;
	float shininess = 128;
} material;

int main() 
{
	GLFWwindow* window = initWindow("Moebius Shader", screenWidth, screenHeight);

	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	Framebuffer FrameBufferObject;
	FrameBufferObject.InitFBO(screenWidth, screenHeight);

	// Shaders
	CrosshatchingShader Crosshatching;
	OutlineShader Outline;
	NoiseShader Noise;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	// Game Loop
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		FrameBufferObject.Render(camera, deltaTime);
		Crosshatching.Render(deltaTime);
		Outline.Render(deltaTime);
		Noise.Render(deltaTime);

		drawUI();

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

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

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