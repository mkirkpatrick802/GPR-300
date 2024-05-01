#pragma once
#include <ew/external/glad.h>

#include "ew/camera.h"
#include "ew/shader.h"

struct FramebufferPackage;

struct CrosshatchingSettings
{
	bool  screen_space_hatching = true;
	int crosshatching_tiling = 10;

	float crosshatching_full_threshold = .75f;
	float crosshatching_half_threshold = .5f;
	float crosshatching_first_threshold = .2f;
};

class CrosshatchingShader
{
public:

	CrosshatchingShader();
	void Create();
	void Render(const ew::Camera& camera, const FramebufferPackage& package, float deltaTime);

public:
	CrosshatchingSettings settings;

private:

	ew::Shader shader;
	GLuint crosshatchingTexture;
};
