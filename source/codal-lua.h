#ifndef CODAL_LUA_H
#define CODAL_LUA_H

extern "C" {
#include "lua.h"
}

void register_lua_api(lua_State *L);

#endif
