#pragma once
#include "OutlineShader.h"

OutlineShader::OutlineShader(): shader(ew::Shader("assets/outline/outline.vert", "assets/outline/outline.frag"))
{

}

void OutlineShader::Render(const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);

	glClear(GL_DEPTH_BUFFER_BIT);

	//Bind Textures
	glBindTextureUnit(0, package.colorBuffer);
	glBindTextureUnit(1, package.normalBuffer);

	shader.use();
	shader.setInt("color_texture", 0);
	shader.setInt("normal_texture", 1);
	shader.setInt("screen_width", package.width);
	shader.setInt("screen_height", package.height);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}