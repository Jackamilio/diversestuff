#include "additional_bindings.h"
#include <LuaBridge/LuaBridge.h>
#include "ImFileDialog.h"
#include "imgui.h"
#include "imgui_lua_bindings.h"
#include "utils.h"

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
}