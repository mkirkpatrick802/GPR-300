#pragma once
#include "ew/camera.h"
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/transform.h"
#include "ew/external/glad.h"

struct FramebufferPackage
{
	unsigned int screenVAO;
	unsigned int colorBuffer;
};

class Framebuffer
{
public:

	Framebuffer();

	void InitFBO(int screenWidth, int screenHeight);
	void Render(const ew::Camera& camera, float deltaTime);

	FramebufferPackage FBOPackage;

private:

	int screenWidth = 1080;
	int screenHeight = 720;

	unsigned int FBO;
	unsigned int ColorBuffer;

	ew::Shader shader;

	ew::Model model;
	GLuint modelTexture;
	ew::Transform modelTransform;

	ew::Mesh plane;
	ew::Transform planeTransform;

	float screenQuad[24] = {

	-1.f,  1.f, 0.f, 1.f,
	-1.f, -1.f, 0.f, 0.f,
	 1.f,  1.f, 1.f, 1.f,

	 1.f,  1.f, 1.f, 1.f,
	-1.f, -1.f, 0.f, 0.f,
	 1.f, -1.f, 1.f, 0.f,
	};

	unsigned int screenVAO;

/*
 *		Shadows
 */

	ew::Shader depthShader;

	const unsigned int SHADOW_WIDTH = 1024;
	const unsigned int SHADOW_HEIGHT = 1024;

	unsigned int depthMapFBO;
	unsigned int depthMap;
};
