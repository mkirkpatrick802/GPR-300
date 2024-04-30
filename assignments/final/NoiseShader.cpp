#include "NoiseShader.h"

NoiseShader::NoiseShader(): shader(ew::Shader("assets/noise/noise.vert", "assets/noise/noise.frag"))
{
}

void NoiseShader::Render(const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);

	//Bind Textures
	//glBindTextureUnit(0, texture);

	shader.use();

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}