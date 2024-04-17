#pragma once
#include "ew/camera.h"
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/transform.h"
#include "ew/external/glad.h"

class Framebuffer
{
public:

	Framebuffer();

	void InitFBO(int screenWidth, int screenHeight);
	void Render(const ew::Camera& camera, float deltaTime);

private:

	unsigned int FBO;
	unsigned int ColorBuffer;
	unsigned int RBO;

	ew::Shader shader;

	ew::Model model;
	GLuint texture;
	ew::Transform modelTransform;
};
