// -*- mode: c++; indent-tabs-mode: nil; -*-
#include <cstdint>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// only defined in Lua version >=5.2
#define LUA_OK 0

// metadata descriptor of the lua-script block in the firmware
struct LuaMeta {
    uint32_t magic;
    uint32_t start;
    uint32_t end;
    uint32_t size;
    uint32_t space;
};

#define LUA_META_MAGIC 0x4C554131

// defined by the linker script
extern const LuaMeta __lua_meta;

#include "MicroBit.h"
#include "MicroBitBLEManager.h"
#include "MicroBitUARTService.h"
#include "codal-lua.h"

MicroBit uBit;

static int lua_panic_handler(lua_State *L) {
    (void)L;
    while (true) {
        uBit.display.scroll("Lua panic");
        uBit.sleep(1000);
    }
}

int main() {
    uBit.init();

    DMESG("main speaking");

    if ((__lua_meta.end - __lua_meta.start != __lua_meta.size)
        || __lua_meta.magic != LUA_META_MAGIC)
    {
        while (true)
        {
            uBit.display.scroll("LuaMeta block problem, aborting");
            uBit.sleep(1000);
        }
    }

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

    if (luaL_loadbuffer(L, (const char*)__lua_meta.start,
                        __lua_meta.size, "embedded") == LUA_OK)
    {
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            const char *err = lua_tostring(L, -1);
            uBit.display.scroll("Lua error!");
            if (err) {
                uBit.display.scroll(err);
            }
        }
    }
    else
    {
        const char* err = lua_tostring(L, -1);
        uBit.display.scroll("Compile error: ");
        if (err) {
            uBit.display.scroll(err);
        }
    }

    lua_close(L);

    uBit.display.scroll("Lua exited");

    release_fiber();
}
