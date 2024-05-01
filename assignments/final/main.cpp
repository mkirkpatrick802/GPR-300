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
#include "ew/texture.h"

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

// Shaders
CrosshatchingShader Crosshatching;
OutlineShader Outline;

int main() 
{
	GLFWwindow* window = initWindow("Moebius Shader", screenWidth, screenHeight);
	glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	setupCamera();

	Framebuffer FrameBufferObject;
	FrameBufferObject.InitFBO(screenWidth, screenHeight);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	camera.farPlane = 30;

	const auto finalShader = ew::Shader("assets/final.vert","assets/final.frag");
	auto noise = ew::loadTexture("assets/noise/noise.jpg");

	Crosshatching.Create();
	Outline.Create();
	NoiseShader Noise;

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
		Crosshatching.Render(camera , FrameBufferObject.FBOPackage, deltaTime);
		Outline.Render(FrameBufferObject.FBOPackage, deltaTime);
		Noise.Render(FrameBufferObject.FBOPackage, deltaTime);

		// Final Render
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, screenWidth, screenHeight);
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTextureUnit(0, FrameBufferObject.FBOPackage.crosshatchingBuffer);
			glBindTextureUnit(1, FrameBufferObject.FBOPackage.outlineBuffer);
			glBindTextureUnit(2, noise);

			finalShader.use();
			finalShader.setInt("crosshatching_texture", 0);
			finalShader.setInt("outline_texture", 1);
			finalShader.setInt("noise_texture", 2);

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

	ImGui::Spacing();
	if (ImGui::Button("Reset Camera")) { resetCamera(); }
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Crosshatching"))
	{
		ImGui::Checkbox("Screen Space", &Crosshatching.settings.screen_space_hatching);
		if(Crosshatching.settings.screen_space_hatching)
			ImGui::InputInt("UV Tiling", &Crosshatching.settings.crosshatching_tiling);

		ImGui::SliderFloat("Full Hatching Threshold", &Crosshatching.settings.crosshatching_full_threshold, 0, 1);
		ImGui::SliderFloat("Half Hatching Threshold", &Crosshatching.settings.crosshatching_half_threshold, 0, 1);
		ImGui::SliderFloat("First Hatching Threshold", &Crosshatching.settings.crosshatching_first_threshold, 0, 1);
	}

	if (ImGui::CollapsingHeader("Outlines"))
	{
		ImGui::SliderFloat("Outline Amount", &Outline.outlineAmount, 0, 10);
		ImGui::SliderFloat3("Outline Color", &Outline.color.x, 0, 1);
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