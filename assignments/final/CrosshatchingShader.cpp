#include "CrosshatchingShader.h"

#include "Framebuffer.h"
#include "ew/texture.h"

CrosshatchingShader::CrosshatchingShader() = default;

void CrosshatchingShader::Create()
{
	shader = ew::Shader("assets/crosshatching/crosshatching.vert", "assets/crosshatching/crosshatching.frag");
	crosshatchingTexture = ew::loadTexture("assets/crosshatching/cross_transparent.png");
}

void CrosshatchingShader::Render(const ew::Camera& camera, const FramebufferPackage& package, float deltaTime)
{
	glBindFramebuffer(GL_FRAMEBUFFER, package.FBO);

	glBindTextureUnit(0, package.shadowBuffer);
	glBindTextureUnit(1, crosshatchingTexture);

	glBindTextureUnit(2, package.colorBuffer);
	glBindTextureUnit(3, package.lightingBuffer);
	glBindTextureUnit(4, package.positionBuffer);

	shader.use();
	shader.setInt("shadow_buffer", 0);
	shader.setInt("crosshatching_texture", 1);
	shader.setInt("color_texture", 2);
	shader.setInt("lighting_texture", 3);
	shader.setInt("position_texture", 4);
	shader.setVec3("camera_position", camera.position);

	//ImGUI Properties
	shader.setInt("screen_space_hatching", settings.screen_space_hatching);
	shader.setInt("crosshatching_tiling", settings.crosshatching_tiling);

	shader.setFloat("crosshatching_full_threshold", settings.crosshatching_full_threshold);
	shader.setFloat("crosshatching_half_threshold", settings.crosshatching_half_threshold);
	shader.setFloat("crosshatching_first_threshold", settings.crosshatching_first_threshold);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
