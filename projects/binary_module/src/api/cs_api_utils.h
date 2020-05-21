#ifndef ZONTWELG_SHARP_MODULE_API_UTILS_H
#define ZONTWELG_SHARP_MODULE_API_UTILS_H

#include "cs_api_common.h"

namespace gm_sharp
{
	extern "C" {
		CS_API_FUN void CS_PushSpecial(int type, bool* result);
		CS_API_FUN void CS_PushString(const char* value, bool* result);
		CS_API_FUN void CS_PushNumber(double value, bool* result);
		CS_API_FUN void CS_PushBool(bool value, bool* result);
		CS_API_FUN void CS_PushNil(bool* result);
		CS_API_FUN void CS_Pop(int offset, bool* result);
		CS_API_FUN void CS_Call(int numArgs, int numResults, bool* result);
		CS_API_FUN void CS_GetField(int offset, const char* name, bool* result);
		CS_API_FUN const char* CS_GetString(int stackPos, bool* result);
		CS_API_FUN double CS_GetNumber(int stackPos, bool* result);
		CS_API_FUN bool CS_GetBool(int stackPos, bool* result);
	}
}

#endif
