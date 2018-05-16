#ifndef ___GRAPHIC_CONTEXT_H___
#define ___GRAPHIC_CONTEXT_H___

#include "Shader.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include <glm/glm.hpp>

#define MAX_LIGHTS 8

class GraphicContext {
public:
	TextureManager<AllegroTexture> textures;
	ProgramManager programs;
	ModelManager models;

	glm::mat4 proj;
	glm::mat4 view;

	glm::vec3 ambient;
	glm::mat2x4 pointLights[MAX_LIGHTS];

	void SetCommonUniforms();

	GraphicContext() : models(*this) {}
	~GraphicContext() { Clear(); }

	void Clear() {
		programs.Clear();
		models.Clear();
		textures.Clear();
	}

private:

};

#endif//___GRAPHIC_CONTEXT_H___
