#include "rcon_detours.h"
#include "utils\utils_ssdk.h"
#include "utils\utils_macro.h"
#include "rcon_signatures.h"
#include "utils\utils_net.h"
#include "shared.h"
#include <Lua/Interface.h>
#include "detours\detours_context.h"

using namespace GarrysMod::Lua;
using namespace std;

static lua_State* s_LuaState;

DETOUR_DECLARE_A4( CServerRemoteAccess_WriteDataRequest, void, CRConServer*, pServerRa, ra_listener_id, listener, const void*, buffer, int, bufferSize);
DETOUR_DECLARE_A1( CServerRemoteAccess_IsPassword, bool, const char*, pPassword);
DETOUR_DECLARE_A2( CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg);

DETOUR_DECLARE_MEMBER_A4(CServerRemoteAccess_WriteDataRequest, void, CRConServer*, pServerRa, ra_listener_id, listener, const void*, buffer, int, bufferSize) {
	// Check that the buffer contains at least the id and type
	if (bufferSize < static_cast<int>(2 * sizeof(int))) {
		return;
	}

	CUtlBuffer cmd(buffer, bufferSize, CUtlBuffer::READ_ONLY);
	_G::g_LastRconAddress = ListenerIdToAddress(reinterpret_cast<CServerRemoteAccess *>(this), listener);
	_G::g_LastListenerId = listener;

	// While there is commands to read
	while (static_cast<int>(cmd.TellGet()) < static_cast<int>(cmd.Size() - 2 * sizeof(int))) {
		// Parse out the buffer
		const auto requestId = cmd.GetInt();
		const auto requestType = cmd.GetInt();

		auto state = s_LuaState;
		auto isNil = true;
		auto hookResult = true;
		char strBuffer[256];
		cmd.GetStringManualCharCount( strBuffer, 256 );
		
		LUA->PushSpecial(SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Call");
			LUA->PushString(GMSVRCON2_HOOK_WRITE_REQUEST);
			LUA->PushNil();
			LUA->PushNumber(listener);
			LUA->PushNumber(requestId);
			LUA->PushNumber(requestType);
			LUA->PushString(strBuffer);
			LUA->PushBool(_G::g_LastRconAddress->IsReservedAdr());
			LUA->PushString(_G::g_LastRconAddress->ToString(true));
			LUA->PushNumber(_G::g_LastRconAddress->GetPort());
			LUA->Call(9, 1);

			if (LUA->GetType(-1) == Type::BOOL) {
				isNil = false;
				hookResult = LUA->GetBool(-1);
			}
		LUA->Pop(3);
		
		if (!isNil && !hookResult){
			return;
		}

		return DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(pServerRa, listener, buffer, bufferSize);
	}

	DETOUR_MEMBER_CALL(CServerRemoteAccess_WriteDataRequest)(pServerRa, listener, buffer, bufferSize);
}

DETOUR_DECLARE_MEMBER_A1(CServerRemoteAccess_IsPassword, bool, const char*, pPassword) {
	if (!_G::g_LastRconAddress) {
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': invalid pointer to g_LastRconAddress!\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
	}

	if (!_G::g_LastRconAddress->IsValid()){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_CHECK_PASSWORD "': network address is invalid\n");
		return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
	}

	auto state = s_LuaState;
	auto isNil = true;
	auto hookResult = true;

	LUA->PushSpecial(SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Call");
		LUA->PushString(GMSVRCON2_HOOK_CHECK_PASSWORD);
		LUA->PushNil();
		LUA->PushString(pPassword);
		LUA->PushNumber(_G::g_LastListenerId);
		LUA->PushBool(_G::g_LastRconAddress->IsReservedAdr());
		LUA->PushString(_G::g_LastRconAddress->ToString(true));
		LUA->PushNumber(_G::g_LastRconAddress->GetPort());
		LUA->Call(7, 1);

		if (LUA->GetType(-1) == Type::BOOL) {
			isNil = false;
			hookResult = LUA->GetBool(-1);
		}
	LUA->Pop(3);

	if (!isNil) {
		return hookResult;
	}

	return DETOUR_MEMBER_CALL(CServerRemoteAccess_IsPassword)(pPassword);
}

DETOUR_DECLARE_MEMBER_A2( CServerRemoteAccess_LogCommand, void, ra_listener_id, listener, const char*, msg) {
	const auto server = reinterpret_cast<CServerRemoteAccess *>(this);
	const auto m_ListenerIDs = reinterpret_cast<CUtlLinkedList<ListenerStore_t, int>*>(reinterpret_cast<unsigned char*>(server) + 0x2C);
	if (!m_ListenerIDs){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': invalid pointer m_ListenerIDs!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	const auto isKnownListener = listener < static_cast<ra_listener_id>(m_ListenerIDs->Count()) && (*m_ListenerIDs)[listener].m_bHasAddress; 
	auto address = (*m_ListenerIDs)[listener].adr;

	if (!address.IsValid()){
		printf("[gmsv_rcon] Skip hook '" GMSVRCON2_HOOK_LOG_COMMAND "': got invalid address!\n");
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
		return;
	}

	auto isNil = true;
	auto hookResult = true;
	auto state = s_LuaState;

	LUA->PushSpecial( SPECIAL_GLOB );
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Call");
		LUA->PushString(GMSVRCON2_HOOK_LOG_COMMAND); 
		LUA->PushNil();
		LUA->PushNumber(listener);
		LUA->PushString(msg);
		LUA->PushBool(isKnownListener);
		if (!isKnownListener){
			LUA->PushBool(false);
			LUA->PushBool(false);
			LUA->PushBool(false);
		} else {
			LUA->PushBool(address.IsReservedAdr());
			LUA->PushString(address.ToString(true));
			LUA->PushNumber(address.GetPort());
		}
		LUA->Call(8, 1);

		if (LUA->GetType(-1) == Type::BOOL) {
			isNil = false;
			hookResult = LUA->GetBool(-1);
		}
	LUA->Pop(3);
		
	if (!isNil && !hookResult){
		DETOUR_MEMBER_CALL(CServerRemoteAccess_LogCommand)(listener, msg);
	}
}

bool DetourRconInit(lua_State* state) {
	s_LuaState = state;

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_WriteDataRequest,
			"CServerRemoteAccess::WriteDataRequest",
			SIG_CSERVERREMOTEACCESS_WRITEDATAREQUEST,
			"engine.dll"
	)

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_IsPassword,
			"CServerRemoteAccess::IsPassword",
			SIG_CSERVERREMOTEACCESS_ISPASSWORD,
			"engine.dll"
	);

	DETOUR_CREATE_MEMBER(
		CServerRemoteAccess_LogCommand,
			"CServerRemoteAccess::LogCommand",
			SIG_CSERVERREMOTEACCESS_LOGCOMMAND,
			"engine.dll"
	);

	printf("[gmsv_rcon2] Rcon detoured!\n");
	return true;
}

bool DetourRconShutdown() {
	DETOUR_DESTROY(CServerRemoteAccess_WriteDataRequest);
	DETOUR_DESTROY(CServerRemoteAccess_IsPassword);
	DETOUR_DESTROY(CServerRemoteAccess_LogCommand);
	return true;
}
