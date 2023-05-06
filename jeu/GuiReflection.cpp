#include "GuiReflection.h"
#include "imgui.h"

void GuiReflection(const char* name, float& value)
{
	ImGui::InputFloat(name, &value);
}

void GuiReflection(const char* name, int& value)
{
	ImGui::InputInt(name, &value);
}

void GuiReflection(const char* name, Vector3& value)
{
	ImGui::InputFloat3(name, &value.x);
}
