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
float height_scale;

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

float transition_threshold = .5f;

void CreateScreenQuad(unsigned int& VAO);

void CreateFrameBuffer(unsigned int& FBO);

int main() {
	GLFWwindow* window = initWindow("Work Session 2", screenWidth, screenHeight);

	ew::Shader water = ew::Shader("assets/water.vert", "assets/water.frag");
	ew::Shader pbr = ew::Shader("assets/pbr.vert", "assets/pbr.frag");
	ew::Shader island = ew::Shader("assets/island.vert", "assets/island.frag");
	ew::Shader transition = ew::Shader("assets/transition.vert", "assets/transition.frag");

	ew::Transform waterTransform;
	GLuint water_texture = ew::loadTexture("assets/water.png");
	GLuint island_heightmap = ew::loadTexture("assets/heightmap.png");
	GLuint gradient = ew::loadTexture("assets/gradient3.png");

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

	ew::Mesh plane = ew::createPlane(512, 512, 2000);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	unsigned int ScreenVAO = 0;
	CreateScreenQuad(ScreenVAO);

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

		// Render Island
		{
			glBindTextureUnit(0, island_heightmap);

			island.use();
			island.setInt("main_texture", 0);
			island.setFloat("height_scale", height_scale);

			island.setMat4("model_matrix", waterTransform.modelMatrix());
			island.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());

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

		// Render Transition 
		{
			glBindTextureUnit(0, gradient);

			transition.use();
			transition.setFloat("threshold", transition_threshold);
			transition.setInt("transition_texture", 0);

			glBindVertexArray(ScreenVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
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

	if (ImGui::CollapsingHeader("Height Map")) {
		ImGui::SliderFloat("Height Scale", &height_scale, 0, 50);
	}

	if (ImGui::CollapsingHeader("Transition")){
		ImGui::SliderFloat("Threshold", &transition_threshold, 0, 1);
	}

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

void CreateFrameBuffer(unsigned int& FBO)
{
	// Create FBO
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create Color Buffer
	unsigned int colorBuffer;
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Bind Color Buffer to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

	// Create RBO
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Bind RBO to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	// Check FBO
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(0 && "Frame Buffer Not Complete");

	// Unbind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateScreenQuad(unsigned int& VAO)
{
	// Create Screen Quad
	float screenQuad[] = {

		-1.f,  1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 0.f,
		 1.f,  1.f, 1.f, 1.f,

		 1.f,  1.f, 1.f, 1.f,
		-1.f, -1.f, 0.f, 0.f,
		 1.f, -1.f, 1.f, 0.f,
	};

	// Create VAO
	glGenVertexArrays(1, &VAO);

	// Create VBO
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// Bind VAO and VBO
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Read Quad Position's and UV's into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), &screenQuad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);
}
