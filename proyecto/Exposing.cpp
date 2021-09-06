#include "Exposing.h"
#include "imgui.h"

void Expose::Value(bool& b, const char* name)
{
	ImGui::Checkbox(name, &b);
}

void Expose::Value(int& integer, const char* name)
{
	ImGui::DragInt(name, &integer);
}

void Expose::Value(float& flt, const char* name)
{
	ImGui::DragFloat(name, &flt);
}

void Expose::Value(std::string& str, const char* name)
{
	static char buffer[512] = "";
	sprintf_s(buffer, 512, str.c_str());
	if (ImGui::InputText(name, buffer, 512)) {
		str = buffer;
	}
}

void Expose::Value(ALLEGRO_BITMAP* img, const char* name)
{
	if (ImGui::TreeNode(name)) {
		ImGui::Image(img, ImVec2(al_get_bitmap_width(img), al_get_bitmap_height(img)));
		ImGui::TreePop();
	}
}
