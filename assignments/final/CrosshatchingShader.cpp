#include "CrosshatchingShader.h"

#include "Framebuffer.h"

CrosshatchingShader::CrosshatchingShader(): shader(ew::Shader("assets/crosshatching/crosshatching.vert", "assets/crosshatching/crosshatching.frag"))
{
}

void CrosshatchingShader::Render(FramebufferPackage package, float deltaTime)
{
	glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	glBindVertexArray(package.screenVAO);
	glBindTexture(GL_TEXTURE_2D, package.colorBuffer);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}