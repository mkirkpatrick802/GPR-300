#pragma once
#include <ew/external/glad.h>
#include "ew/shader.h"

struct FramebufferPackage;

class CrosshatchingShader
{
public:

	CrosshatchingShader();
	void Render(const FramebufferPackage& package, float deltaTime);

private:

	ew::Shader shader;
	GLuint crosshatchingTexture;
};
