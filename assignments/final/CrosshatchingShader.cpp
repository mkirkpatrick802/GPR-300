#include "CrosshatchingShader.h"

#include "Framebuffer.h"
#include "ew/texture.h"

CrosshatchingShader::CrosshatchingShader(): shader(ew::Shader("assets/crosshatching/crosshatching.vert", "assets/crosshatching/crosshatching.frag")), crosshatchingTexture(ew::loadTexture("assets/crosshatching/CrosshatchingTexture.png"))

{
}

void CrosshatchingShader::Render(FramebufferPackage package, float deltaTime)
{
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTextureUnit(0, package.colorBuffer);
	glBindTextureUnit(1, package.shadowMap);
	glBindTextureUnit(2, crosshatchingTexture);

	shader.use();
	shader.setInt("screen_texture", 0);
	shader.setInt("shadow_map", 1);
	shader.setInt("crosshatching_texture", 2);

	glBindVertexArray(package.screenVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}