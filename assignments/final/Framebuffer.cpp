#include "Framebuffer.h"

#include <cassert>
#include <cstddef>

#include "ew/procGen.h"
#include "ew/texture.h"
#include "ew/external/glad.h"

Framebuffer::Framebuffer(): FBOPackage(), FBO(0), ColorBuffer(0),
                            shader(ew::Shader("assets/lit.vert", "assets/lit.frag")),
                            model(ew::Model("assets/suzanne.obj")), modelTexture(ew::loadTexture("assets/brick_color.jpg")),
                            plane(ew::createPlane(305, 305, 128)), screenVAO(0), depthShader(ew::Shader("assets/depth.vert", "assets/depth.frag"))
{
	planeTransform.position = glm::vec3(0, -2, 0);
}

void Framebuffer::InitFBO(int screenWidth, int screenHeight)
{
/*
 *		FBO
 */

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

/*
 *		Screen Quad
 */

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

/*
 *		Shadows
 */

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

	FBOPackage.colorBuffer = ColorBuffer;
	FBOPackage.screenVAO = screenVAO;
}

void Framebuffer::Render(const ew::Camera& camera, const float deltaTime)
{
	const float near_plane = 1.0f;
	const float far_plane = 7.5f;
	const glm::vec3 lightPosition = glm::vec3(-3, 3, -3);
	const glm::mat4 lightProjection = glm::ortho(lightPosition.x, lightPosition.y, lightPosition.z, 3.0f, near_plane, far_plane);
	const glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));

	// 1. Render to Depth Map
	{
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

		glCullFace(GL_FRONT);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);

		depthShader.use();
		depthShader.setMat4("projection_matrix", lightSpaceMatrix);
		depthShader.setMat4("model_matrix", modelTransform.modelMatrix());
		model.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// 2. Render Scene as Normal with Shadow Mapping
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glCullFace(GL_BACK);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Bind textures
		glBindTextureUnit(0, modelTexture);
		glBindTextureUnit(1, depthMap);

		// Set uniforms
		shader.use();

		// Vertex Shader
		shader.setMat4("projection_matrix", camera.projectionMatrix());
		shader.setMat4("view_matrix", camera.viewMatrix());
		shader.setMat4("lightspace_matrix", lightSpaceMatrix);

		// Fragment Shader
		shader.setInt("diffuse_texture", 0);
		shader.setInt("shadow_map", 1);
		shader.setVec3("light_position", lightPosition);
		shader.setVec3("view_position", camera.position);

		// Render Scene
		shader.setMat4("model_matrix", modelTransform.modelMatrix());
		model.draw();

		shader.setMat4("model_matrix", planeTransform.modelMatrix());
		plane.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}