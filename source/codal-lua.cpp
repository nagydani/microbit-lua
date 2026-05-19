// -*- mode: c++; indent-tabs-mode: nil; -*-
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "MicroBit.h"

extern MicroBit uBit;

#define LUA_FUNCTION_LIST					\
    X(sleep,  { int ms = (int)luaL_checkinteger(L, 1);		\
                uBit.sleep(ms);					\
                return 0;					\
              })						\
    X(scroll, { uBit.display.scroll(luaL_checkstring(L, 1));	\
                return 0;					\
              })

#define X(name, body) static int l_##name(lua_State *L) body
LUA_FUNCTION_LIST
#undef X

#define X(name, body) {#name, l_##name},
static const luaL_Reg l_microbit[] = {
    LUA_FUNCTION_LIST
    {NULL, NULL}
};
#undef X

void register_lua_api(lua_State *L) {
  luaL_register(L, "microbit", l_microbit);
}
