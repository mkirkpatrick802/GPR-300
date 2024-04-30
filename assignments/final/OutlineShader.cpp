#pragma once
#include "OutlineShader.h"

OutlineShader::OutlineShader(): shader(ew::Shader("assets/outline/outline.vert", "assets/outline/outline.frag"))
{

}

void OutlineShader::Render(const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);

	//Bind Textures
	//glBindTextureUnit(0, texture);

	shader.use();

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}