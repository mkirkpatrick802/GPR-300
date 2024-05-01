#pragma once
#include "Framebuffer.h"
#include "ew/shader.h"


class OutlineShader
{

public:

	OutlineShader();
	void Render(const FramebufferPackage& package, float deltaTime);
	void Create();

	glm::vec3 color;
	float outlineAmount = 1.0f;

private:
	ew::Shader shader;

};