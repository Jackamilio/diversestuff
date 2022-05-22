#pragma once

#include "lua/lua.hpp"

namespace ImGui {
	namespace LuaBindings {
		void Load(lua_State* lState);
		bool CleanEndStack();
	}
}
