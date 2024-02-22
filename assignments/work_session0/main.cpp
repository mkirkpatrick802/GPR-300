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

#include "ew/procGen.h"

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

struct Material {
	float ka = 1.0;
	float kd = 0.5;
	float ks = 0.5;
	float shininess = 128;
}material;

struct PBRMaterial {
	GLuint color;
	GLuint albedo;
	GLuint metallic;
	GLuint occulusion;
	GLuint specular;
} pbr_material;

int main() {
	GLFWwindow* window = initWindow("Work Session 0", screenWidth, screenHeight);

	ew::Shader water = ew::Shader("assets/water.vert", "assets/water.frag");
	ew::Shader pbr = ew::Shader("assets/pbr.vert", "assets/pbr.frag");

	ew::Transform waterTransform;
	GLuint water_texture = ew::loadTexture("assets/water.png");

	ew::Transform shellTransform;
	ew::Model model = ew::Model("assets/greenshell/greenshell.obj");
	pbr_material.color = ew::loadTexture("assets/greenshell/greenshell_col.png");
	pbr_material.albedo = ew::loadTexture("assets/greenshell/greenshell_ao.png");
	pbr_material.metallic = ew::loadTexture("assets/greenshell/greenshell_mtl.png");
	pbr_material.occulusion = ew::loadTexture("assets/greenshell/greenshell_rgh.png");
	pbr_material.specular = ew::loadTexture("assets/greenshell/greenshell_spc.png");

	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60;

	ew::Mesh plane = ew::createPlane(512, 512, 128);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	// Game Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		waterTransform.position = glm::vec3(0, -10, 0);

		shellTransform.rotation = glm::rotate(shellTransform.rotation, deltaTime, glm::vec3(0, 1, 0));
		shellTransform.position = glm::vec3(0, 2, -8);

		// Render Water
		{
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Bind textures
			glBindTextureUnit(0, water_texture);

			// Set uniforms
			water.use();
			water.setInt("main_texture", 0);
			water.setFloat("time", time);

			water.setMat4("model_matrix", waterTransform.modelMatrix());
			water.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());

			plane.draw();
		}

		// Render Shell
		{

			// Bind textures
			glBindTextureUnit(0, pbr_material.color);
			glBindTextureUnit(1, pbr_material.albedo);
			glBindTextureUnit(2, pbr_material.metallic);
			glBindTextureUnit(3, pbr_material.occulusion);
			glBindTextureUnit(4, pbr_material.specular);

			// Set uniforms
			pbr.use();
			pbr.setInt("materials.color", 0);
			pbr.setInt("materials.albedo", 1);
			pbr.setInt("materials.metallic", 2);
			pbr.setInt("materials.occulusion", 3);
			pbr.setInt("materials.specular", 4);
			pbr.setVec3("eye_position", camera.position);
			pbr.setVec3("ambient_color", ambient_color);

			pbr.setMat4("model_matrix", shellTransform.modelMatrix());
			pbr.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());

			model.draw();
		}

		drawUI();

		glfwSwapBuffers(window);
	}

	printf("Shutting down...");
}

void resetCamera() {
	camera.position = glm::vec3(0, 0, 5.0f);
	camera.target = glm::vec3(0);
	cameraController.yaw = cameraController.pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");

	if (ImGui::Button("Reset Camera")) {
		resetCamera();
	}

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 1024.0f);
	}

	ImGui::Text("Add Controls Here!");
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
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
GLFWwindow* initWindow(const char* title, int width, int height) {
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

