#include <lua.hpp>
#include "lua_stack_ops.h"
#include "lua_state.h"

using namespace Halley;

LuaStackOps::LuaStackOps(LuaState& state)
	: state(state)
{
}

void LuaStackOps::push(std::nullptr_t)
{
	lua_pushnil(state.getRawState());
}

void LuaStackOps::push(bool v)
{
	lua_pushboolean(state.getRawState(), v);
}

void LuaStackOps::push(int v)
{
	lua_pushinteger(state.getRawState(), v);
}

void LuaStackOps::push(int64_t v)
{
	lua_pushinteger(state.getRawState(), v);
}

void LuaStackOps::push(double v)
{
	lua_pushnumber(state.getRawState(), v);
}

void LuaStackOps::push(const String& v)
{
	lua_pushstring(state.getRawState(), v.c_str());
}

void LuaStackOps::push(Vector2i v)
{
	lua_createtable(state.getRawState(), 0, 2);
	push(v.x);
	lua_setfield(state.getRawState(), -2, "x");
	push(v.y);
	lua_setfield(state.getRawState(), -2, "y");
}

void LuaStackOps::pop()
{
	lua_pop(state.getRawState(), 1);
}

bool LuaStackOps::popBool()
{
	auto value = lua_toboolean(state.getRawState(), -1) != 0;
	pop();
	return value;
}

int LuaStackOps::popInt()
{
	auto value = lua_tointeger(state.getRawState(), -1);
	pop();
	return int(value);
}

int64_t LuaStackOps::popInt64()
{
	auto value = lua_tointeger(state.getRawState(), -1);
	pop();
	return value;
}

double LuaStackOps::popDouble()
{
	auto value = lua_tonumber(state.getRawState(), -1);
	pop();
	return value;
}

String LuaStackOps::popString()
{
	String value = lua_tostring(state.getRawState(), -1);
	pop();
	return value;
}

Vector2i LuaStackOps::popVector2i()
{
	if (!lua_istable(state.getRawState(), -1)) {
		throw Exception("Invalid value at Lua stack");
	}
	Vector2i result;
	lua_getfield(state.getRawState(), -1, "x");
	result.x = popInt();
	lua_getfield(state.getRawState(), -1, "y");
	result.y = popInt();
	return result;
}

LuaStackReturn::LuaStackReturn(LuaState& state)
	: state(state)
{
}

LuaStackReturn::operator void() const
{
}

LuaStackReturn::operator bool() const
{
	return LuaStackOps(state).popBool();
}

LuaStackReturn::operator int() const
{
	return LuaStackOps(state).popInt();
}

LuaStackReturn::operator int64_t() const
{
	return LuaStackOps(state).popInt64();
}

LuaStackReturn::operator double() const
{
	return LuaStackOps(state).popDouble();
}

LuaStackReturn::operator String() const
{
	return LuaStackOps(state).popString();
}

LuaStackReturn::operator Vector2i() const
{
	return LuaStackOps(state).popVector2i();
}