extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "MicroBit.h"

// only defined in Lua version >=5.2
#define LUA_OK 0

MicroBit uBit;

/* -----------------------------
   Lua -> CODAL bridge function
   ----------------------------- */

static int l_scroll(lua_State *L) {
    uBit.display.scroll(luaL_checkstring(L, 1));
    return 0;
}

/* Register functions into Lua */
static void register_lua_api(lua_State *L) {
    lua_register(L, "scroll", l_scroll);
}

static void idle_loop() {
  while (1) {
    uBit.sleep(1000);
  }
}

static int lua_panic_handler(lua_State *L) {
    (void)L;
    while (1) {
        uBit.display.scroll("Lua panic");
    }
}

int main() {
    uBit.init();

    /* Create Lua state using CODAL heap */
    lua_State *L = luaL_newstate();
    if (!L) {
        lua_panic_handler(NULL);
    }

    lua_atpanic(L, lua_panic_handler);

    /* Load standard libraries */
    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);

    register_lua_api(L);

    /* Example Lua script controlling LEDs */
    const char *script =
        "scroll('Lua speaking')\n"
        "scroll('...')\n";

    if (luaL_dostring(L, script) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        if (err) {
            uBit.display.scroll("Lua error");
        }
    }

    lua_close(L);

    uBit.display.scroll("Lua exited");

    idle_loop();
}
