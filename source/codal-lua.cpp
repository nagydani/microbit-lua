// -*- mode: c++; indent-tabs-mode: nil; -*-
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "MicroBit.h"

extern MicroBit uBit;



#define LUA_MICROBIT_FUNCTIONS						\
    X(reset,      { uBit.reset();					\
                    return 0;						\
                  })							\
    X(sleep,      { uint32_t ms = (uint32_t)luaL_checkinteger(L, 1);	\
                    uBit.sleep(ms);					\
                    return 0;						\
                  })							\
    X(seedRandom, { uint32_t seed = (uint32_t)luaL_optinteger(L, 1, 0);	\
                    if(seed) { uBit.seedRandom(seed); }			\
                    else { uBit.seedRandom(); }				\
                    return 0;						\
		  })							\
    X(random,     { int max = (int)luaL_checkinteger(L, 1);		\
                    lua_pushinteger(L, (lua_Integer)uBit.random(max));	\
                    return 1;						\
                  })							\
    X(systemTime, { lua_pushinteger(L, (lua_Integer)uBit.systemTime());	\
                    return 1;						\
                  })							\
    X(friendlyName, { lua_pushstring(L, microbit_friendly_name());	\
                    return 1;						\
                  })							\
    X(panic,      { int statusCode = (int)luaL_checkinteger(L, 1);      \
                    microbit_panic(statusCode);				\
                    return 0;						\
                  })

#define LUA_DISPLAY_FUNCTIONS						\
    X(scroll,     { uBit.display.scroll(luaL_checkstring(L, 1));	\
                    return 0;						\
                  })

// I failed counting X expansions in LUA_DISPLAY_FUNCTIONS automatically
#define LUA_DISPLAY_COUNT 1

#define X(name, body) static int l_##name(lua_State *L) body
LUA_MICROBIT_FUNCTIONS
LUA_DISPLAY_FUNCTIONS
#undef X

#define X(name, body) {#name, l_##name},
static const luaL_Reg l_microbit[] = {
    LUA_MICROBIT_FUNCTIONS
    {NULL, NULL}
};
static const luaL_Reg l_display[] = {
    LUA_DISPLAY_FUNCTIONS
    {NULL, NULL}
};
#undef X

void register_lua_api(lua_State *L) {
  luaL_register(L, "microbit", l_microbit);
  lua_createtable(L, 0, LUA_DISPLAY_COUNT);
  luaL_register(L, NULL, l_display);
  lua_setfield(L, -2, "display");
}
