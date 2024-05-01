#pragma once
#include "OutlineShader.h"

OutlineShader::OutlineShader(): shader(ew::Shader("assets/outline/outline.vert", "assets/outline/outline.frag"))
{

}

void OutlineShader::Render(const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);
	glClearColor(1, 1, 1, 1);
	glClear( GL_DEPTH_BUFFER_BIT);
	//Bind Textures
	glBindTextureUnit(0, package.colorBuffer);
	glBindTextureUnit(1, package.normalBuffer);

	shader.use();
	shader.setInt("_ColorBuffer", 0);
	shader.setVec3("_OutlineColor", glm::vec3(0));
	shader.setInt("_NormalBuffer", 1);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}