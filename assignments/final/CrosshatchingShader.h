#pragma once
#include <ew/external/glad.h>
#include "ew/shader.h"

class CrosshatchingShader
{
public:

	CrosshatchingShader();
	void Render(float deltaTime);

private:

	ew::Shader shader;
	GLuint crosshatchingTexture;
};