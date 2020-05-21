#include <Lua/Interface.h>
#include <cstdlib>
#include "rcon_detours.h"
#include <cstdio>
#include "utils/utils_ssdk.h"
#include "utils/utils_memory.h"
#include "rcon_signatures.h"
#include "config/module_config.h"
#include "shared.h"
#include "sharp_runtime.h"
#include "utils/utils_validating.h"
using namespace GarrysMod::Lua;

int LuaFnCsExecInlineStr(lua_State* state);

GMOD_MODULE_OPEN() {
	_G::g_LuaState = state;

	printf("[gmsv_sharp] Initializing ...\n");
	ConVar_Register();

	auto tierLib = VStdLib_GetICVarFactory();
	ConnectTier1Libraries(&tierLib, 1);
	ConnectTier2Libraries(&tierLib, 1);
	ConnectTier3Libraries(&tierLib, 1);

	if (!gm_sharp::ModuleConfig::Initialize()) {
		printf("[gmsv_sharp] Error loading module configuration\n");
		return EXIT_FAILURE;
	}

	_G::g_SharpRuntime = new gm_sharp::Runtime(
		gm_sharp::ModuleConfig::GetClrRootPath(),
		gm_sharp::ModuleConfig::GetClrLibsPath(),
		gm_sharp::ModuleConfig::GetApiLibsPath(),
		gm_sharp::ModuleConfig::GetApiCoreLibName(), 
		gm_sharp::ModuleConfig::GetApiModulesLookupPath());

	long resultOrErr;
	if (!_G::g_SharpRuntime->Initialize(&resultOrErr)) {
		printf("[gmsv_sharp] Error initializing sharp runtime %ld\n", resultOrErr);
		return EXIT_FAILURE;
	}

	LUA->PushSpecial( SPECIAL_GLOB );
		LUA->PushCFunction(LuaFnCsExecInlineStr);
		LUA->SetField(-2, "CS_ExecInlineStr");
	LUA->Pop();

	printf("[gmsv_sharp] Loading done\n");
	return EXIT_SUCCESS;
}

GMOD_MODULE_CLOSE() {
	if (IsValidPtr(_G::g_SharpRuntime)) {
		_G::g_SharpRuntime->Shutdown();
	}

	_G::g_LuaState = nullptr;
	return EXIT_SUCCESS;
}

int LuaFnCsExecInlineStr(lua_State* state) {
	if (!IsValidPtr(_G::g_SharpRuntime)) {
		LUA->PushBool(false);
		LUA->PushString("SharpRuntime are nullptr");
		return 2;
	}

	LUA->CheckString(1);
	const auto str = LUA->GetString(1);
	const auto ret = _G::g_SharpRuntime->CompileInline(std::string(str));
	printf("[gmsv_sharp] CompileInline done, %d\n", ret);
	LUA->PushBool(true);
	LUA->PushNumber(ret);
	return 2;
}