#include "GraphicContext.h"

GraphicContext::GraphicContext() :
	models(*this),
	proj(1.0f),
	view(1.0f),
	ambient(0.1f),
	pointLights()
{}

GraphicContext::~GraphicContext() { Clear(); }

void GraphicContext::Clear() {
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
