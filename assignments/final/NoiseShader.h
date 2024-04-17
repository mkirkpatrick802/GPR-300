#pragma once
#include <ew/external/glad.h>
#include "ew/shader.h"

class NoiseShader
{
public:

	NoiseShader();
	void Render(float deltaTime);

private:

	ew::Shader shader;
	GLuint noiseTexture;
};