#pragma once
#include <ew/external/glad.h>

#include "Framebuffer.h"
#include "ew/shader.h"

class NoiseShader
{
public:

	NoiseShader();
	void Render(const FramebufferPackage& package, float deltaTime);

private:

	ew::Shader shader;
	GLuint noiseTexture;
};