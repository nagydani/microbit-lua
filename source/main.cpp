extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "MicroBit.h"

MicroBit uBit;

/* -----------------------------
   Lua -> CODAL bridge function
   ----------------------------- */

/*
 * scroll(text)
 * Lua usage: scroll("hello")
 */
static int l_scroll(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);

    if (text) {
        uBit.display.scroll(text);
    }

    return 0;
}

/* Panic handler */
static int panic(lua_State *L) {
    (void)L;
    while (1);
    return 0;
}

/* Register functions into Lua */
static void register_lua_api(lua_State *L) {
    lua_register(L, "scroll", l_scroll);
}

/* -----------------------------
   Main entry point
   ----------------------------- */

int main() {
    uBit.init();

    /* Create Lua state using CODAL heap */
    lua_State *L = luaL_newstate();
    if (!L) {
        while (1);
    }

    lua_atpanic(L, panic);

    /* Load standard libraries */
    luaopen_base(L);
    luaopen_table(L);
    luaopen_string(L);
    luaopen_math(L);

    /* Register CODAL API */
    register_lua_api(L);

    /* Example Lua script controlling LEDs */
    const char *script =
        "scroll('Hello micro:bit')\n"
        "scroll('Lua + CODAL')\n";

    if (luaL_loadstring(L, script) || lua_pcall(L, 0, 0, 0)) {
        const char *err = lua_tostring(L, -1);
        if (err) {
            uBit.display.scroll("Lua error");
        }
    }

    lua_close(L);

    uBit.display.scroll("Lua closed");

    /* Idle loop */
    while (1) {
        uBit.sleep(1000);
    }
}
