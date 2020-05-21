#include "shared.h"
#include "utils/utils_ssdk.h"
#include "../sharp_runtime.h"

netadr_s* _G::g_LastRconAddress = nullptr;
ra_listener_id _G::g_LastListenerId = 0;
gm_sharp::Runtime* _G::g_SharpRuntime = nullptr;
lua_State* _G::g_LuaState = nullptr;