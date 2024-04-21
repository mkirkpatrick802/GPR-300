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
	auto suzanne = new Model(ew::Model("assets/suzanne.obj"), ew::Transform(), ew::loadTexture("assets/brick_color.jpg"));

	ew::Transform planeT;
	planeT.position = glm::vec3(0, -2, 0);
	auto plane = new Model(ew::createPlane(305, 305, 128), planeT, ew::loadTexture("assets/brick_color.jpg"));

	sceneModels.push_back(suzanne);
	sceneModels.push_back(plane);
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

		// Create Color Buffer
		glGenTextures(1, &ColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Color Buffer to FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0);

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

		FBOPackage.colorBuffer = ColorBuffer;
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
		glGenFramebuffers(1, &shadowMapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

		// Create Shadow Map
		glGenTextures(1, &shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowMap);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowMap, 0);

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

		FBOPackage.shadowMap = shadowMap;
	}
}

void Framebuffer::Render(const ew::Camera& camera, const float deltaTime)
{
	const float near_plane = 1.0f;
	const float far_plane = 7.5f;
	const glm::vec3 lightPosition = glm::vec3(-3, 3, -3);
	const glm::mat4 lightProjection = glm::ortho(lightPosition.x, lightPosition.y, lightPosition.z, 3.0f, near_plane, far_plane);
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

		RenderScene(depthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// 2. Render to Shadow Map
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);

		glCullFace(GL_BACK);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
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
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind textures
		glBindTextureUnit(0, sceneModels[0]->modelTexture);

		// Set uniforms
		litShader.use();

		// Vertex Shader
		litShader.setMat4("projection_matrix", camera.projectionMatrix());
		litShader.setMat4("view_matrix", camera.viewMatrix());

		// Fragment Shader
		litShader.setInt("diffuse_texture", 0);
		litShader.setVec3("light_position", lightPosition);
		litShader.setVec3("view_position", camera.position);

		RenderScene(litShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Framebuffer::RenderModel(ew::Shader shader, Model* model)
{
	shader.setMat4("model_matrix", model->modelTransform.modelMatrix());

	if (model->model.isValid)
	{
		model->model.draw();
	}
	else
	{
		model->mesh.draw();
	}
}

void Framebuffer::RenderScene(ew::Shader shader)
{
	for (const auto model : sceneModels)
	{
		shader.setMat4("model_matrix", model->modelTransform.modelMatrix());

		if (model->model.isValid)
		{
			model->model.draw();
		}
		else
		{
			model->mesh.draw();
		}
	}
}
