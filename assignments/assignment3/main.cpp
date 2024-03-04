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
#include <random>
#include <GL/gl.h>

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

struct ScreenQuad
{
	unsigned int vao;
	unsigned int vbo;
}quad;

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

float screenQuad[] = {

	-1.f,  1.f, 0.f, 1.f,
	-1.f, -1.f, 0.f, 0.f,
	 1.f,  1.f, 1.f, 1.f,

	 1.f,  1.f, 1.f, 1.f,
	-1.f, -1.f, 0.f, 0.f,
	 1.f, -1.f, 1.f, 0.f,
};

void CreateScreenQuad()
{
	// Create VAO
	glGenVertexArrays(1, &quad.vao);

	// Create VBO
	glGenBuffers(1, &quad.vbo);

	// Bind VAO and VBO
	glBindVertexArray(quad.vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);

	// Read Quad Position's and UV's into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), &screenQuad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);
}

struct PointLight {

	glm::vec3 position;
	float radius = 8;
	glm::vec3 color = glm::vec3(1, 0, 1);
};

const int MAX_POINT_LIGHTS = 100;
PointLight pointLights[MAX_POINT_LIGHTS];

float generateRandomFloat(float min, float max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(min, max);
	return dis(gen);
}

int main()
{
	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);

	ew::Shader gPass = ew::Shader("assets/gPass.vert", "assets/gPass.frag");
	ew::Shader litPass = ew::Shader("assets/litPass.vert", "assets/litPass.frag");
	ew::Shader lightOrb = ew::Shader("assets/lightOrb.vert", "assets/lightOrb.frag");

	ew::Mesh sphereMesh = ew::Mesh(ew::createSphere(1.0f, 8));

	ew::Model model = ew::Model("assets/suzanne.obj");
	ew::Transform modelTransform;

	ew::Mesh plane = ew::createPlane(305, 305, 128);
	ew::Transform planeTransform;
	planeTransform.position = glm::vec3(-150, -1, -150);

	GLuint paperTexture = ew::loadTexture("assets/texture.jpg");
	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");

	camera.position = glm::vec3(3, 0, 3);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	camera.fov = 60;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	CreateDeferredPass();
	CreateScreenQuad();

	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		pointLights[i].position.x = generateRandomFloat(-100, 0);
		pointLights[i].position.y = generateRandomFloat(1, 3);
		pointLights[i].position.z = generateRandomFloat(-100, 0);
		pointLights[i].color = glm::vec4(generateRandomFloat(0, 1), generateRandomFloat(0, 1), generateRandomFloat(0, 1), 1);
	}

	// Render Settings
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

			glEnable(GL_DEPTH_TEST);
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Bind textures
			glBindTextureUnit(0, paperTexture);
			glBindTextureUnit(1, brickTexture);

			// Set uniforms
			gPass.use();
			gPass.setInt("main_texture", 0);
			gPass.setInt("uv_scale", 1);

			for (int x = 0; x <= 100; x++)
			{
				for (int z = 0; z <= 100; z++)
				{
					modelTransform.position.x = (float)(x * 4) * -1;
					modelTransform.position.z = (float)(z * 4) * -1;
					gPass.setMat4("model_matrix", modelTransform.modelMatrix());
					model.draw();
				}
			}

			gPass.setInt("main_texture", 1);
			gPass.setInt("uv_scale", 20);
			gPass.setMat4("model_matrix", planeTransform.modelMatrix());
			plane.draw();

			gPass.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Lighting Pass
		{
			glDisable(GL_DEPTH_TEST);
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindTextureUnit(0, deffered.position);
			glBindTextureUnit(1, deffered.normal);
			glBindTextureUnit(2, deffered.albedo);
			glBindTextureUnit(3, deffered.depth);

			litPass.use();
			litPass.setInt("positionTex", 0);
			litPass.setInt("normalTex", 1);
			litPass.setInt("albedoTex", 2);
			litPass.setInt("depthTex", 3);

			litPass.setVec3("eye_position", camera.position);
			litPass.setVec3("ambient_color", ambient_color);
			litPass.setFloat("material.ka", material.ka);
			litPass.setFloat("material.kd", material.kd);
			litPass.setFloat("material.ks", material.ks);
			litPass.setFloat("material.shininess", material.shininess);

			for (int i = 0; i < MAX_POINT_LIGHTS; i++)
			{
				std::string prefix = "point_lights[" + std::to_string(i) + "].";
				litPass.setVec3(prefix + "position", pointLights[i].position);
				litPass.setVec3(prefix + "color", pointLights[i].color);
				litPass.setFloat(prefix + "radius", pointLights[i].radius);
			}

			glBindVertexArray(quad.vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// Sphere Pass
		{
			glEnable(GL_DEPTH_TEST);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, deffered.fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			//Draw all light orbs
			lightOrb.use();
			lightOrb.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			for (int i = 0; i < MAX_POINT_LIGHTS; i++)
			{
				glm::mat4 m = glm::mat4(1.0f);
				m = glm::translate(m, pointLights[i].position);
				m = glm::scale(m, glm::vec3(0.2f)); //Whatever radius you want

				lightOrb.setMat4("_Model", m);
				lightOrb.setVec3("_Color", pointLights[i].color);
				sphereMesh.draw();
			}
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