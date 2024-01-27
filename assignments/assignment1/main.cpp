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

struct PostProcessEffect
{
	int renderScaler = 1;
	bool invertColor = false;
	bool depthEdgeDetection = false;
	bool grayscaleEdge = false;
}PPE;

int main() 
{
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Shader postShader = ew::Shader("assets/post.vert", "assets/post.frag");

	ew::Model model = ew::Model("assets/suzanne.obj");
	ew::Transform modelTransform;

	GLuint texture = ew::loadTexture("assets/texture.jpg");

	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = static_cast<float>(screenWidth) / screenHeight;
	camera.fov = 60;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// Create FBO
	unsigned int FBO;
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
	unsigned int screenVAO;
	glGenVertexArrays(1, &screenVAO);

	// Create VBO
	unsigned int screenVBO;
	glGenBuffers(1, &screenVBO);

	// Bind VAO and VBO
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);

	// Read Quad Position's and UV's into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), &screenQuad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);

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

		// Render Frame Buffer
		{
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);

			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cameraController.move(window, &camera, deltaTime);

			modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));

			// Bind textures
			glBindTextureUnit(0, texture);

			// Set uniforms
			litShader.use();
			litShader.setInt("main_texture", 0);
			litShader.setVec3("eye_position", camera.position);
			litShader.setVec3("ambient_color", ambient_color);
			litShader.setFloat("material.ka", material.ka);
			litShader.setFloat("material.kd", material.kd);
			litShader.setFloat("material.ks", material.ks);
			litShader.setFloat("material.shininess", material.shininess);

			litShader.setMat4("model_matrix", modelTransform.modelMatrix());
			litShader.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());
			model.draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Render Post Process Shader
		{
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			postShader.use();
			postShader.setFloat("screen_width", static_cast<float>(screenWidth));
			postShader.setFloat("screen_height", static_cast<float>(screenHeight));
			postShader.setFloat("time", deltaTime);

			postShader.setInt("render_count", PPE.renderScaler);
			postShader.setFloat("invert_color", PPE.invertColor);
			postShader.setFloat("depth_edge_detection", PPE.depthEdgeDetection);
			postShader.setFloat("grayscale_edge", PPE.grayscaleEdge);

			glBindVertexArray(screenVAO);
			glBindTexture(GL_TEXTURE_2D, colorBuffer);

			glDrawArrays(GL_TRIANGLES, 0, 6);
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

	if (ImGui::CollapsingHeader("Post Process")) {

		ImGui::SliderInt("Render Scaler", &PPE.renderScaler, 1, 10);
		ImGui::Checkbox("Invert Colors", &PPE.invertColor);
		ImGui::Checkbox("Sobel Depth Edge Detection", &PPE.depthEdgeDetection);
		if(PPE.depthEdgeDetection) {

			ImGui::Checkbox("Edge Detection Grayscaled", &PPE.grayscaleEdge);
		}
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

