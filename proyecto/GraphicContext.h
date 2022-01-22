#ifndef ___GRAPHIC_CONTEXT_H___
#define ___GRAPHIC_CONTEXT_H___

#include "Shader.h"
#include "TextureManager.h"
#include "ModelManager.h"

#define MAX_LIGHTS 8

class GraphicContext {
public:
	TextureManager textures;
	ProgramManager programs;
	ModelManager models;

	glm::mat4 proj;
	glm::mat4 view;

	glm::vec3 ambient;
	glm::mat2x4 pointLights[MAX_LIGHTS];

	GraphicContext();
	~GraphicContext();

	void Clear();

	void SetCommonUniforms();

private:

};

#endif//___GRAPHIC_CONTEXT_H___
