#include "cs_api_utils.h"
#include "../utils/utils_validating.h"
#include "../shared.h"

#define LUA _G::g_LuaState->luabase
#define SET_RESULT(x) *result = x

void gm_sharp::CS_PushSpecial(int type, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->PushSpecial(type);
	SET_RESULT(true);
}

void gm_sharp::CS_PushString(const char* value, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->PushString(value);
	SET_RESULT(true);
}

void gm_sharp::CS_PushNumber(double value, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->PushNumber(value);
	SET_RESULT(true);
}

void gm_sharp::CS_PushBool(bool value, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->PushBool(value);
	SET_RESULT(true);
}

void gm_sharp::CS_PushNil(bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->PushNil();
	SET_RESULT(true);
}

void gm_sharp::CS_Pop(int offset, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->Pop(offset);
	SET_RESULT(true);
}

void gm_sharp::CS_Call(int numArgs, int numResults, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	LUA->Call(numArgs, numResults);
	SET_RESULT(true);
}

void gm_sharp::CS_GetField(int offset, const char* name, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return;
	}

	if (!name) {
		SET_RESULT(false);
		return;
	}

	LUA->GetField(offset, name);
	SET_RESULT(true);
}

const char* gm_sharp::CS_GetString(int stackPos, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return nullptr;
	}

	const auto ret = LUA->GetString(stackPos);
	SET_RESULT(true);
	return ret;
}

double gm_sharp::CS_GetNumber(int stackPos, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return 0;
	}

	const auto ret = LUA->GetNumber(stackPos);
	SET_RESULT(true);
	return ret;
}

bool gm_sharp::CS_GetBool(int stackPos, bool* result) {
	if (!IsValidPtr(_G::g_LuaState)) {
		SET_RESULT(false);
		return 0;
	}

	const auto ret = LUA->GetBool(stackPos);
	SET_RESULT(true);
	return ret;
}
