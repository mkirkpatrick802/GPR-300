#pragma once
#include "ew/camera.h"
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/transform.h"
#include "ew/external/glad.h"

struct FramebufferPackage
{
	unsigned int screenVAO;
	unsigned int FBO;
	unsigned int colorBuffer;
	unsigned int lightingBuffer;
	unsigned int depthMap;
	unsigned int normalBuffer;
	unsigned int finalBuffer;

	unsigned int shadowBuffer;
};

struct Model
{
	Model(const ew::Model& m, const ew::Transform& t, const GLuint tx): model(m), modelTransform(t), modelTexture(tx) {}
	Model(const ew::Mesh& m, const ew::Transform& t, const GLuint tx): mesh(m), modelTransform(t), modelTexture(tx) {}

	ew::Model model;
	ew::Mesh mesh;

	ew::Transform modelTransform;
	GLuint modelTexture;
};

class Framebuffer
{
public:

	Framebuffer();

	void InitFBO(int screenWidth, int screenHeight);
	void Render(const ew::Camera& camera, float deltaTime);

	FramebufferPackage FBOPackage;

private:

	void RenderModel(ew::Shader shader, Model* model);
	void RenderScene(ew::Shader shader, bool texture = false);

private:


	int screenWidth = 1080;
	int screenHeight = 720;

/*
 *		Scene
 */

	unsigned int FBO;
	unsigned int FinalBuffer;
	unsigned int PositionBuffer;
	unsigned int NormalBuffer;
	unsigned int ColorBuffer;
	unsigned int LightingBuffer;

	ew::Shader litShader;

	std::vector<Model*> sceneModels;

/*
 *		PPE
 */

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
	ew::Shader shadowShader;

	const unsigned int SHADOW_WIDTH = 4048;
	const unsigned int SHADOW_HEIGHT = 4048;

	unsigned int depthMapFBO;
	unsigned int depthMap;

	unsigned int shadowBufferFBO;
	unsigned int shadowBuffer;
};
