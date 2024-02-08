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

struct Material {
	float ka = 0.5;
	float kd = 0.5;
	float ks = 0.5;
	float shininess = 128;
}material;

struct PostProcessEffect {
	int renderScaler = 1;
	bool invertColor = false;
	bool depthEdgeDetection = false;
	bool grayscaleEdge = false;
}PPE;

int main() {

	GLFWwindow* window = initWindow("Assignment 2", screenWidth, screenHeight);

	ew::Shader litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	//ew::Shader postShader = ew::Shader("assets/post.vert", "assets/post.frag");
	ew::Shader depthShader = ew::Shader("assets/depth.vert", "assets/depth.frag");
	ew::Shader debugShader = ew::Shader("assets/debug.vert", "assets/debug.frag");
	ew::Shader shadowShader = ew::Shader("assets/shadow.vert", "assets/shadow.frag");

	ew::Model model = ew::Model("assets/suzanne.obj");
	ew::Transform modelTransform;

	GLuint texture = ew::loadTexture("assets/texture.jpg");

	camera.position = glm::vec3(0, 0, 5);
	camera.target = glm::vec3(0, 0, 0);
	camera.aspectRatio = static_cast<float>(screenWidth) / screenHeight;
	camera.fov = 60;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	float planeVertices[]
	{
		+25.f, -1.5f, -25.f, 0.f, 1.f, 0.f, 0.f, 25.f,
		-25.f, -1.5f, +25.f, 0.f, 1.f, 0.f, 0.f, 0.f,
		+25.f, -1.5f, +25.f, 0.f, 1.f, 0.f, 25.f, 0.f,

		+25.f, -1.5f, -25.f, 0.f, 1.f, 0.f, 25.f, 25.f,
		-25.f, -1.5f, -25.f, 0.f, 1.f, 0.f, 0.f, 25.f,
		-25.f, -1.5f, +25.f, 0.f, 1.f, 0.f, 25.f, 0.f
	};

	/*
	*	Create Plane
	*/

	// Create VAO
	unsigned int planeVAO;
	glGenVertexArrays(1, &planeVAO);

	// Create VBO
	unsigned int planeVBO;
	glGenBuffers(1, &planeVBO);

	// Bind VAO and VBO
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	// Read Quad Position's and UV's into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), static_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);

	/*
	*	Depth Map
	*/

	// Create Depth Map FBO
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// Create Depth Map
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Bind Depth Map to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		assert(0 && "Frame Buffer Not Complete");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*
	*	Debug Quad
	*/

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
	unsigned int debugVAO;
	glGenVertexArrays(1, &debugVAO);

	// Create VBO
	unsigned int debugVBO;
	glGenBuffers(1, &debugVBO);

	// Bind VAO and VBO
	glBindVertexArray(debugVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugVBO);

	// Read Quad Position's and UV's into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), &screenQuad, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Unbind VAO
	glBindVertexArray(0);

	/*
	 *	Render Loop
	*/

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

		cameraController.move(window, &camera, deltaTime);

		float near_plane = 1.0f, far_plane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, near_plane, far_plane);
		glm::mat4 lightView = lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));

		// Render to Depth FBO
		{
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

			// Pass
			glClear(GL_DEPTH_BUFFER_BIT);

			// Pipeline
			glViewport(0, 0, 1024, 1024);

			// Bind
			depthShader.use();
			depthShader.setMat4("model_matrix", modelTransform.modelMatrix());
			depthShader.setMat4("projection_matrix", lightSpaceMatrix);

			// Draw
			model.draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		// Render Suzanna on main screen
		{
			// Pass
			glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Pipeline
			glViewport(0, 0, screenWidth, screenHeight);
 
			// Bind
			shadowShader.use();
			shadowShader.setMat4("projection", camera.projectionMatrix());
			shadowShader.setMat4("view", camera.viewMatrix());
			shadowShader.setVec3("view_position", camera.position);
			shadowShader.setVec3("light_position", glm::vec3(-3.0f, 3.0f, -3.0f));
			shadowShader.setMat4("light_matrix", lightSpaceMatrix);
			shadowShader.setMat4("model", modelTransform.modelMatrix());

			shadowShader.setVec3("ambient_color", ambient_color);
			shadowShader.setFloat("material.ka", material.ka);
			shadowShader.setFloat("material.kd", material.kd);
			shadowShader.setFloat("material.ks", material.ks);
			shadowShader.setFloat("material.shininess", material.shininess);

			glBindTextureUnit(0, texture);
			shadowShader.setInt("diffuse_texture", 0);

			glBindTextureUnit(1, depthMap);
			shadowShader.setInt("shadow_map", 1);

			// Draw
			model.draw();

		}

		// Render Plane on main screen
		{
			// Pass

			// Pipeline
			glViewport(0, 0, screenWidth, screenHeight);

			// Bind
			shadowShader.use();
			shadowShader.setMat4("projection", camera.projectionMatrix());
			shadowShader.setMat4("view", camera.viewMatrix());
			shadowShader.setVec3("view_position", camera.position);
			shadowShader.setVec3("light_position", glm::vec3(-3.0f, 3.0f, -3.0f));
			shadowShader.setMat4("light_matrix", lightSpaceMatrix);
			shadowShader.setMat4("model", glm::mat4(1));

			shadowShader.setVec3("ambient_color", ambient_color);
			shadowShader.setFloat("material.ka", material.ka);
			shadowShader.setFloat("material.kd", material.kd);
			shadowShader.setFloat("material.ks", material.ks);
			shadowShader.setFloat("material.shininess", material.shininess);

			glBindTextureUnit(0, texture);
			shadowShader.setInt("diffuse_texture", 0);

			glBindTextureUnit(1, depthMap);
			shadowShader.setInt("shadow_map", 1);

			glBindVertexArray(planeVAO);

			// Draw
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// Render Debug Square w/ Depth FBO
		{
			// Pass

			// Pipeline
			glViewport(screenWidth - 150, 0, 150, 150);

			// Bind
			glBindVertexArray(debugVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);

			debugShader.use();
			debugShader.setInt("debug_img", 0);

			// Draw
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

