#include "Framebuffer.h"

#include <cassert>
#include <cstddef>

#include "ew/texture.h"
#include "ew/external/glad.h"

Framebuffer::Framebuffer(): FBO(0), ColorBuffer(0), RBO(0), shader(ew::Shader("assets/lit.vert", "assets/lit.frag")),
                            model(ew::Model("assets/suzanne.obj")), texture(ew::loadTexture("assets/brick_color.jpg"))
{

}

void Framebuffer::InitFBO(int screenWidth, int screenHeight)
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

void Framebuffer::Render(const ew::Camera& camera, const float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelTransform.rotation = glm::rotate(modelTransform.rotation, deltaTime, glm::vec3(0, 1, 0));

	// Bind textures
	glBindTextureUnit(0, texture);

	// Set uniforms
	shader.use();
	shader.setInt("main_texture", 0);
	shader.setVec3("eye_position", camera.position);
	shader.setVec3("ambient_color", glm::vec3(0, 0, 0));
	shader.setFloat("material.ka", 1.0);
	shader.setFloat("material.kd", 0.5);
	shader.setFloat("material.ks", 0.5);
	shader.setFloat("material.shininess", 128);

	shader.setMat4("model_matrix", modelTransform.modelMatrix());
	shader.setMat4("projection_matrix", camera.projectionMatrix() * camera.viewMatrix());
	model.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}