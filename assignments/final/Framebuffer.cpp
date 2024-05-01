#include "Framebuffer.h"

#include <cassert>
#include <cstddef>

#include "ew/procGen.h"
#include "ew/texture.h"
#include "ew/external/glad.h"

Framebuffer::Framebuffer(): FBOPackage(), FBO(0), ColorBuffer(0),
                            litShader(ew::Shader("assets/lit.vert", "assets/lit.frag")), screenVAO(0),
                            depthShader(ew::Shader("assets/depth.vert", "assets/depth.frag")),
							shadowShader(ew::Shader("assets/shadow.vert", "assets/shadow.frag"))
{
	auto suzanne = new Model(ew::Model("assets/suzanne.obj"), ew::Transform(), ew::loadTexture("assets/paper.jpg"));

	ew::Transform sphereTransform;
	sphereTransform.position = glm::vec3(-3, 0, 0);
	auto sphere = new Model(ew::Model("assets/sphere.obj"), sphereTransform, ew::loadTexture("assets/paper.jpg"));

	ew::Transform planeT;
	planeT.position = glm::vec3(0, -2, 0);
	auto plane = new Model(ew::createPlane(305, 305, 128), planeT, ew::loadTexture("assets/paper.jpg"));

	sceneModels.push_back(suzanne);
	sceneModels.push_back(sphere);
	sceneModels.push_back(plane);

	FBOPackage.width = screenWidth;
	FBOPackage.height = screenHeight;
}

void Framebuffer::InitFBO(int screenWidth, int screenHeight)
{
/*
 *		FBO
 */

	{
		// Create FBO
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		// Create Position Buffer
		glGenTextures(1, &PositionBuffer);
		glBindTexture(GL_TEXTURE_2D, PositionBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Position to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PositionBuffer, 0);

		// Create Position Buffer
		glGenTextures(1, &NormalBuffer);
		glBindTexture(GL_TEXTURE_2D, NormalBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Position to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, NormalBuffer, 0);

		// Create Color Buffer
		glGenTextures(1, &ColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Color Buffer to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, ColorBuffer, 0);

		// Create Lighting Buffer
		glGenTextures(1, &LightingBuffer);
		glBindTexture(GL_TEXTURE_2D, LightingBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Lighting Buffer to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, LightingBuffer, 0);

		// Create Final Buffer
		glGenTextures(1, &FinalBuffer);
		glBindTexture(GL_TEXTURE_2D, FinalBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Final to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, FinalBuffer, 0);

		// Create RBO
		unsigned int RBO;
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Bind RBO to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		const int num = 5;
		const unsigned int attachments[num] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(num, attachments);

		// Check FBO
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			assert(0 && "Frame Buffer Not Complete");

		// Unbind FBO
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FBOPackage.FBO = FBO;
		FBOPackage.finalBuffer = FinalBuffer;
		FBOPackage.colorBuffer = ColorBuffer;
		FBOPackage.lightingBuffer = LightingBuffer;
		FBOPackage.normalBuffer = NormalBuffer;
	}

/*
 *		Screen Quad
 */

	{
		// Create VAO
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

		FBOPackage.screenVAO = screenVAO;
	}

/*
 *		Create Shadow Depth Map FBO
 */
	{
		glGenFramebuffers(1, &depthMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			assert(0 && "Frame Buffer Not Complete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

/*
 *		Create Shadow Map FBO
 */

	{
		glGenFramebuffers(1, &shadowBufferFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferFBO);

		// Create Shadow Map
		glGenTextures(1, &shadowBuffer);
		glBindTexture(GL_TEXTURE_2D, shadowBuffer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowBuffer, 0);

		// Create RBO
		unsigned int RBO;
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Bind RBO to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			assert(0 && "Frame Buffer Not Complete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FBOPackage.shadowBuffer = shadowBuffer;
	}
}

void Framebuffer::Render(const ew::Camera& camera, const float deltaTime)
{
	const float near_plane = 1.0f;
	const float far_plane = 7.5f;
	const glm::vec3 lightPosition = glm::vec3(-20, 50, -20);
	const glm::mat4 lightProjection = glm::ortho(lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.y, near_plane, far_plane);
	const glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	//modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));

	// 1. Render to Depth Map
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		glCullFace(GL_FRONT);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);

		depthShader.use();
		depthShader.setMat4("projection_matrix", lightSpaceMatrix);

		for (const auto model : sceneModels)
			RenderModel(depthShader, model);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// 2. Render to Shadow Map
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowBufferFBO);

		glCullFace(GL_BACK);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind textures
		glBindTextureUnit(0, depthMap);

		// Set Uniforms
		shadowShader.use();

		// Vertex Shader
		shadowShader.setMat4("projection_matrix", camera.projectionMatrix());
		shadowShader.setMat4("view_matrix", camera.viewMatrix());
		shadowShader.setMat4("lightspace_matrix", lightSpaceMatrix);

		// Fragment Shader
		shadowShader.setInt("depth_map", 0);
		shadowShader.setVec3("light_position", lightPosition);
		shadowShader.setVec3("view_position", camera.position);

		RenderScene(shadowShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// 3. Render w/ lighting
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glCullFace(GL_BACK);
		glViewport(0, 0, screenWidth, screenHeight);

		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set uniforms
		litShader.use();

		// Vertex Shader
		litShader.setMat4("projection_matrix", camera.projectionMatrix());
		litShader.setMat4("view_matrix", camera.viewMatrix());

		// Fragment Shader
		litShader.setVec3("light_position", lightPosition);
		litShader.setVec3("view_position", camera.position);

		RenderScene(litShader, true);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Framebuffer::RenderModel(const ew::Shader shader, Model* model)
{
	shader.setMat4("model_matrix", model->modelTransform.modelMatrix());
	model->model.isValid ? model->model.draw() : model->mesh.draw();
}

void Framebuffer::RenderScene(const ew::Shader shader, const bool texture)
{
	for (const auto model : sceneModels)
	{
		shader.setMat4("model_matrix", model->modelTransform.modelMatrix());

		if(texture)
		{
			glBindTextureUnit(0, model->modelTexture);
			litShader.setInt("diffuse_texture", 0);
		}

		model->model.isValid ? model->model.draw() : model->mesh.draw();
	}
}
