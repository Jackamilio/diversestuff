#include "GraphicContext.h"

void GraphicContext::InitOverlayTransforms()
{
	std::stack<ALLEGRO_TRANSFORM> ot;
	overlayTransforms.swap(ot);
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	overlayTransforms.push(t);
}

GraphicContext::GraphicContext() :
	models(*this),
	proj(1.0f),
	view(1.0f),
	ambient(0.1f),
	pointLights()
{
	InitOverlayTransforms();
}

GraphicContext::~GraphicContext() { Clear(); }

void GraphicContext::Clear() {
	InitOverlayTransforms();
	programs.Clear();
	models.Clear();
	textures.Clear();
}

void GraphicContext::SetCommonUniforms()
{
	Program* program = programs.GetCurrent();
	if (program) {
		program->SetUniform("trFinal", proj * view);
		program->SetUniform("texMain", 0);
		program->SetUniform("lAmbient", ambient);
		program->SetUniform("lpointLights", pointLights, MAX_LIGHTS);
	}
}

void GraphicContext::PushOverlayTransform()
{
	overlayTransforms.push(*al_get_current_transform());
}

void GraphicContext::PopOverlayTransform()
{
	overlayTransforms.pop();
	al_use_transform(&CurrentOverlayTransform());
}

void GraphicContext::IdentityOverlayTransform()
{
	al_identity_transform(&CurrentOverlayTransform());
	al_use_transform(&CurrentOverlayTransform());
}

void GraphicContext::TranslateOverlayTransform(const glm::ivec2& offset)
{
	al_translate_transform(&CurrentOverlayTransform(), offset.x, offset.y);
	al_use_transform(&CurrentOverlayTransform());
}
