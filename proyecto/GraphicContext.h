#ifndef ___GRAPHIC_CONTEXT_H___
#define ___GRAPHIC_CONTEXT_H___

#include "Shader.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include <glm/glm.hpp>
#include <stack>

#define MAX_LIGHTS 8

class GraphicContext {
private:
	std::stack<ALLEGRO_TRANSFORM> overlayTransforms;
	void InitOverlayTransforms();

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

	void PushOverlayTransform();
	void PopOverlayTransform();
	void TranslateOverlayTransform(const glm::ivec2& offset);
	inline ALLEGRO_TRANSFORM& CurrentOverlayTransform() { return overlayTransforms.top(); }

private:

};

#endif//___GRAPHIC_CONTEXT_H___
