#include "additional_bindings.h"
#include <LuaBridge/LuaBridge.h>
#include "ImFileDialog.h"
#include "imgui.h"

std::string imgui_inputtext(const char* label, std::string text) {
	static char buf[128];
	sprintf_s(buf, 128, text.c_str());
	ImGui::InputText(label, buf, 128);
	text = buf;
	return text;
}

void additional_bindings(lua_State* L)
{
	luabridge::getGlobalNamespace(L).beginNamespace("imgui")
		.addFunction("InputText", imgui_inputtext)
		.beginClass<ImGuiIO>("IO")
			.addProperty("KeyCtrl", &ImGuiIO::KeyCtrl)
			.addProperty("KeyShift", &ImGuiIO::KeyShift)
			.addProperty("KeyAlt", &ImGuiIO::KeyAlt)
		.endClass()
		.addFunction("GetIO", ImGui::GetIO)
		.beginClass<ifd::FileDialog>("FileDialog")
			.addFunction("Save", &ifd::FileDialog::Save)
			.addFunction("Open", &ifd::FileDialog::Open)
			.addFunction("IsDone", &ifd::FileDialog::IsDone)
			.addFunction("HasResult", &ifd::FileDialog::HasResult)
			.addFunction("Close", &ifd::FileDialog::Close)
			.addFunction("GetResult", &ifd::FileDialog::GetStrLocalResult)
		.endClass()
		.addFunction("GetFileDialog", ifd::FileDialog::Instance)
		.endNamespace();
}