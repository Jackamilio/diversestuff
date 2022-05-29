#include "additional_bindings.h"
#include <LuaBridge/LuaBridge.h>
#include "ImFileDialog.h"
#include "imgui.h"
#include "imgui_lua_bindings.h"
#include "utils.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

std::string imgui_inputtext(const char* label, std::string text) {
	char buf[128];
	sprintf_s(buf, 128, text.c_str());
	ImGui::InputText(label, buf, 128);
	text = buf;
	return text;
}

bool imgui_io_mousedown(ImGuiIO* io, int b) {
	return io && b >= 0 && b <= 5 && io->MouseDown[b];
}

ImVec2 imgui_imvec2_add(ImVec2* a, const ImVec2& b)
{
	return ImVec2(a->x + b.x, a->y + b.y);
}

ImVec2 imgui_imvec2_sub(ImVec2* a, const ImVec2& b)
{
	return ImVec2(a->x - b.x, a->y - b.y);
}

void additional_bindings(lua_State* L)
{
	luabridge::getGlobalNamespace(L)
		.addFunction("fileexists", fileexists)
		.beginNamespace("imgui")
		.addFunction("CleanEndStack", ImGui::LuaBindings::CleanEndStack)
		.addFunction("InputText", imgui_inputtext)
		.beginClass<ImVec2>("ImVec2")
		.addConstructor<void(*)(float, float)>()
		.addProperty("x", &ImVec2::x)
		.addProperty("y", &ImVec2::y)
		.addFunction("__add", imgui_imvec2_add)
		.addFunction("__sub", imgui_imvec2_sub)
		.endClass()
		.beginClass<ImGuiIO>("ImGuiIO")
		.addProperty("MousePos", &ImGuiIO::MousePos)
		.addFunction("MouseDown", imgui_io_mousedown)
		.addProperty("MouseWheel", &ImGuiIO::MouseWheel)
		.addProperty("MouseWheelH", &ImGuiIO::MouseWheelH)
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
		//.beginNamespace("al")
		//	.addFunction("draw_line", al_draw_line)
		//.endNamespace();
}