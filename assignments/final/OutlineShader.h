#pragma once
#include "ew/shader.h"

class OutlineShader
{
public:

	OutlineShader();
	void Render(float deltaTime);

private:
	ew::Shader shader;

};