#include <stdio.h>
#include <math.h>

// EW
#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>

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

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

// Camera
ew::CameraController cameraController;
ew::Camera camera;

void setupCamera();
void resetCamera();

int main() 
{
	GLFWwindow* window = initWindow("Moebius Shader", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	setupCamera();

	Framebuffer FrameBufferObject;
	FrameBufferObject.InitFBO(screenWidth, screenHeight);

	// Shaders
	CrosshatchingShader Crosshatching;
	OutlineShader Outline;
	NoiseShader Noise;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	camera.farPlane = 30;

	const auto finalShader = ew::Shader("assets/final.vert","assets/final.frag");

	// Game Loop
	while (!glfwWindowShouldClose(window)) 
	{
		glfwPollEvents();

		// Time Keeper
		const float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		// Camera Movement
		cameraController.move(window, &camera, deltaTime);

		// Render Scene
		FrameBufferObject.Render(camera, deltaTime);
		Crosshatching.Render(FrameBufferObject.FBOPackage, deltaTime);
		Outline.Render(FrameBufferObject.FBOPackage, deltaTime);
		Noise.Render(FrameBufferObject.FBOPackage, deltaTime);

		// Final Render
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, screenWidth, screenHeight);
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTextureUnit(0, FrameBufferObject.FBOPackage.finalBuffer);

			finalShader.use();
			finalShader.setInt("final_buffer", 0);

			glBindVertexArray(FrameBufferObject.FBOPackage.screenVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// Render UI
		drawUI();

		// Buffer Swap
		glfwSwapBuffers(window);
	}

	printf("Shutting down...");
}

void setupCamera()
{
	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = (float)screenWidth / (float)screenHeight;
	camera.fov = 60;
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