#include "CrosshatchingShader.h"

#include "Framebuffer.h"
#include "ew/texture.h"

CrosshatchingShader::CrosshatchingShader(): shader(ew::Shader("assets/crosshatching/crosshatching.vert", "assets/crosshatching/crosshatching.frag")), crosshatchingTexture(ew::loadTexture("assets/crosshatching/cross_transparent.png"))
{ }

void CrosshatchingShader::Render(FramebufferPackage package, float deltaTime)
{
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTextureUnit(0, package.positionBuffer);
	glBindTextureUnit(1, package.normalBuffer);
	glBindTextureUnit(2, package.colorBuffer);
	glBindTextureUnit(3, package.shadowBuffer);
	glBindTextureUnit(4, crosshatchingTexture);

	shader.use();
	shader.setInt("position_buffer", 0);
	shader.setInt("normal_buffer", 1);
	shader.setInt("color_buffer", 2);
	shader.setInt("shadow_buffer", 3);
	shader.setInt("crosshatching_texture", 4);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}