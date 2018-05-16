#include "GraphicContext.h"

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
