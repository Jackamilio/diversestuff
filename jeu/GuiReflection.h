#pragma once

#include "imgui.h"
#include "raylib.h"
#include "json.hpp"

void GuiReflection(const char* name, float& value);
void GuiReflection(const char* name, int& value);
void GuiReflection(const char* name, Vector3& value);

// stealing the expand macro from NLOHMANN, thanks bud

#define GUI_REFLECTION_INTERNAL(v1) GuiReflection(#v1, value.v1);
#define GUI_REFLECTION(name, ...) \
	inline void GuiReflection(const char* n, name& value) { \
		if (ImGui::TreeNode(#name)) { \
			NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(GUI_REFLECTION_INTERNAL, __VA_ARGS__)) \
			ImGui::TreePop(); \
		}\
	}

#define JSON_REFLECTION NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT

#define JSONANDGUI_REFLECTION(name, ...) GUI_REFLECTION(name, __VA_ARGS__) JSON_REFLECTION(name, __VA_ARGS__)