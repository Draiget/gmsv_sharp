#ifndef ZONTWELG_GMSVRCON2_SHARED_H
#define ZONTWELG_GMSVRCON2_SHARED_H

#include <Lua/Interface.h>

// Forward declarations of SSDK stuff
typedef struct netadr_s netadr_t;
typedef unsigned int ra_listener_id;
namespace gm_sharp
{
	class Runtime;
}

class _G
{
public:
	static netadr_s* g_LastRconAddress;
	static ra_listener_id g_LastListenerId;
	static gm_sharp::Runtime* g_SharpRuntime;
	static lua_State* g_LuaState;
};

#endif
