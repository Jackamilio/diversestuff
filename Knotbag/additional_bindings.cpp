#include "additional_bindings.h"
#include <LuaBridge/LuaBridge.h>
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
	// for convenience
	luaL_dostring(L, "imgui.Text = imgui.TextUnformatted imgui.Button = imgui.SmallButton");
	luabridge::getGlobalNamespace(L).beginNamespace("imgui").addFunction("InputText", imgui_inputtext).endNamespace();
}