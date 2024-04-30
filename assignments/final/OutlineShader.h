#pragma once
#include "Framebuffer.h"
#include "ew/shader.h"

class OutlineShader
{
public:

	OutlineShader();
	void Render(const FramebufferPackage& package, float deltaTime);

private:
	ew::Shader shader;

};