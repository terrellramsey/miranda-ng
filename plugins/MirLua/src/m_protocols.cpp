#include "stdafx.h"

HANDLE hRecvMessage = NULL;

static int lua_GetProtocol(lua_State *L)
{
	const char *name = NULL;

	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
	{
		const char *proto = GetContactProto(lua_tonumber(L, 1));
		PROTOACCOUNT *pa = Proto_GetAccount(proto);
		if (pa)
			name = pa->szProtoName;
	}
		break;
	case LUA_TSTRING:
		name = lua_tostring(L, 1);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	PROTOCOLDESCRIPTOR *pd = Proto_IsProtocolLoaded(name);
	if (pd)
		MT<PROTOCOLDESCRIPTOR>::Set(L, pd);
	else
		lua_pushnil(L);

	return 1;
}

static int lua_ProtocolIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	int count = lua_tointeger(L, lua_upvalueindex(2));
	PROTOCOLDESCRIPTOR **protos = (PROTOCOLDESCRIPTOR**)lua_touserdata(L, lua_upvalueindex(3));

	if (i < count)
	{
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		MT<PROTOCOLDESCRIPTOR>::Set(L, protos[i]);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Protocols(lua_State *L)
{
	int count;
	PROTOCOLDESCRIPTOR **protos;
	Proto_EnumProtocols(&count, &protos);

	lua_pushinteger(L, 0);
	lua_pushinteger(L, count);
	lua_pushlightuserdata(L, protos);
	lua_pushcclosure(L, lua_ProtocolIterator, 3);

	return 1;
}

static int lua_CallService(lua_State *L)
{
	const char *module = NULL;

	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		module = GetContactProto(lua_tonumber(L, 1));
	break;
	case LUA_TSTRING:
		module = lua_tostring(L, 1);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	const char *service = luaL_checkstring(L, 2);
	WPARAM wParam = (WPARAM)luaM_tomparam(L, 3);
	LPARAM lParam = (LPARAM)luaM_tomparam(L, 4);

	INT_PTR res = CallProtoService(module, service, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ChainSend(lua_State *L)
{
	MCONTACT hContact = luaL_checknumber(L, 1);
	const char *service = luaL_checkstring(L, 2);
	WPARAM wParam = (WPARAM)luaM_tomparam(L, 3);
	LPARAM lParam = (LPARAM)luaM_tomparam(L, 4);

	INT_PTR res = ProtoChainSend(hContact, service, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ChainRecv(lua_State *L)
{
	MCONTACT hContact = luaL_checknumber(L, 1);
	const char *service = luaL_checkstring(L, 2);
	WPARAM wParam = (WPARAM)luaM_tomparam(L, 3);
	LPARAM lParam = (LPARAM)luaM_tomparam(L, 4);

	INT_PTR res = ProtoChainRecv(hContact, service, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

/***********************************************/

static int lua_GetAccount(lua_State *L)
{
	const char *name = NULL;

	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		name = GetContactProto(lua_tonumber(L, 1));
		break;
	case LUA_TSTRING:
		name = lua_tostring(L, 1);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	PROTOACCOUNT *pa = Proto_GetAccount(name);
	if (pa)
		MT<PROTOACCOUNT>::Set(L, pa);
	else
		lua_pushnil(L);

	return 1;
}

static int lua_AccountIterator(lua_State *L)
{
	int i = lua_tointeger(L, lua_upvalueindex(1));
	int count = lua_tointeger(L, lua_upvalueindex(2));
	PROTOACCOUNT **accounts = (PROTOACCOUNT**)lua_touserdata(L, lua_upvalueindex(3));

	if (i < count)
	{
		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		MT<PROTOACCOUNT>::Set(L, accounts[i]);
	}
	else
		lua_pushnil(L);

	return 1;
}

static int lua_Accounts(lua_State *L)
{
	int count;
	PROTOACCOUNT **accounts;
	Proto_EnumAccounts(&count, &accounts);

	lua_pushinteger(L, 0);
	lua_pushinteger(L, count);
	lua_pushlightuserdata(L, accounts);
	lua_pushcclosure(L, lua_AccountIterator, 3);

	return 1;
}

/***********************************************/

INT_PTR FilterRecvMessage(WPARAM wParam, LPARAM lParam)
{
	int res = NotifyEventHooks(hRecvMessage, wParam, lParam);
	if (res) return res;
	Proto_ChainRecv(wParam, (CCSDATA*)lParam);
	return 0;
}

/***********************************************/

static luaL_Reg protocolsApi[] =
{
	{ "GetProtocol", lua_GetProtocol },
	{ "Protocols", lua_Protocols },
	{ "CallService", lua_CallService },
	{ "CallSendChain", lua_ChainSend },
	{ "CallReceiveChain", lua_ChainRecv },

	{ "GetAccount", lua_GetAccount },
	{ "Accounts", lua_Accounts },

	{ NULL, NULL }
};

/***********************************************/

LUAMOD_API int luaopen_m_protocols(lua_State *L)
{
	luaL_newlib(L, protocolsApi);

	MT<PROTOCOLDESCRIPTOR>(L, "PROTOCOLDESCRIPTOR")
		.Field(&PROTOCOLDESCRIPTOR::szName, "Name", LUA_TSTRINGA)
		.Field(&PROTOCOLDESCRIPTOR::type, "Type", LUA_TINTEGER);

	MT<PROTOACCOUNT>(L, "PROTOACCOUNT")
		.Field(&PROTOACCOUNT::szModuleName, "ModuleName", LUA_TSTRINGA)
		.Field(&PROTOACCOUNT::tszAccountName, "AccountName", LUA_TSTRINGW)
		.Field(&PROTOACCOUNT::szProtoName, "ProtoName", LUA_TSTRINGA)
		.Field(&PROTOACCOUNT::bIsEnabled, "IsEnabled", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bIsVisible, "IsVisible", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bIsVirtual, "IsVirtual", LUA_TBOOLEAN)
		.Field(&PROTOACCOUNT::bOldProto, "IsOldProto", LUA_TBOOLEAN);

	MT<ACKDATA>(L, "ACKDATA")
		.Field(&ACKDATA::szModule, "Module", LUA_TSTRINGA)
		.Field(&ACKDATA::hContact, "hContact", LUA_TINTEGER)
		.Field(&ACKDATA::type, "Type", LUA_TINTEGER)
		.Field(&ACKDATA::result, "Result", LUA_TINTEGER)
		.Field(&ACKDATA::hProcess, "hProcess", LUA_TLIGHTUSERDATA)
		.Field(&ACKDATA::lParam, "lParam", LUA_TLIGHTUSERDATA);

	MT<CCSDATA>(L, "CCSDATA")
		.Field(&CCSDATA::hContact, "hContact", LUA_TINTEGER)
		.Field(&CCSDATA::szProtoService, "Service", LUA_TSTRINGA)
		.Field([](CCSDATA *ccd) -> MTFieldVal { MTFieldVal tmp; tmp.userdata = (void*)ccd->wParam; return tmp; }, "wParam", LUA_TLIGHTUSERDATA)
		.Field([](CCSDATA *ccd) -> MTFieldVal { MTFieldVal tmp; tmp.userdata = (void*)ccd->lParam; return tmp; }, "lParam", LUA_TLIGHTUSERDATA);
	
	MT<PROTORECVEVENT>(L, "PROTORECVEVENT")
		.Field(&PROTORECVEVENT::timestamp, "Timestamp", LUA_TINTEGER)
		.Field(&PROTORECVEVENT::flags, "Flags", LUA_TINTEGER)
		.Field(&PROTORECVEVENT::szMessage, "Message", LUA_TSTRING);

	return 1;
}
