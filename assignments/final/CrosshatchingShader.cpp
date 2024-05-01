#include "CrosshatchingShader.h"

#include "Framebuffer.h"
#include "ew/texture.h"

CrosshatchingShader::CrosshatchingShader(): shader(ew::Shader("assets/crosshatching/crosshatching.vert", "assets/crosshatching/crosshatching.frag")),
											crosshatchingTexture(ew::loadTexture("assets/crosshatching/cross_transparent.png"))
{ }

void CrosshatchingShader::Render(const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);

	glBindTextureUnit(0, package.shadowBuffer);
	glBindTextureUnit(1, crosshatchingTexture);

	glBindTextureUnit(2, package.normalBuffer);
	glBindTextureUnit(3, package.colorBuffer);
	glBindTextureUnit(4, package.lightingBuffer);

	shader.use();
	shader.setInt("shadow_buffer", 0);
	shader.setInt("crosshatching_texture", 1);

	shader.setInt("normal_texture", 2);
	shader.setInt("color_texture", 3);
	shader.setInt("lighting_texture", 4);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}