extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// only defined in Lua version >=5.2
#define LUA_OK 0

// these come from the linker, but we need to define them
extern const unsigned char _binary_source_lua_script_lua_start[];
extern const unsigned char _binary_source_lua_script_lua_end[];
// size is weird, so we don't use it (its address is the value...)
// extern const unsigned char _binary_source_lua_script_lua_size[];

#include "MicroBit.h"
#include "MicroBitBLEManager.h"
#include "MicroBitUARTService.h"

MicroBit uBit;

// BLE manager (initializes SoftDevice / BLE stack)
MicroBitBLEManager bleManager();

// UART service (Nordic UART over BLE)
MicroBitUARTService *uart;

void onBLEConnected(MicroBitEvent)
{
    uBit.display.print("C");  // Connected
}

void onBLEDisconnected(MicroBitEvent)
{
    uBit.display.print("D");  // Disconnected
}

void onDataReceived(MicroBitEvent)
{
    // Read incoming BLE data (RX characteristic)
    ManagedString msg = uart->readUntil("\n");

    // Echo to LED display
    uBit.display.scrollAsync(msg);

    // Echo back to laptop (TX notify)
    uart->send("echoing: " + msg);
}

/* -----------------------------
   Lua -> CODAL bridge function
   ----------------------------- */

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

#define X(name, body) lua_register(L, #name, l_##name);
static void register_lua_api(lua_State *L) {
    LUA_FUNCTION_LIST
}
#undef X

static int lua_panic_handler(lua_State *L) {
    (void)L;
    while (1) {
        uBit.display.scroll("Lua panic");
    }
}

void setup_ble_echo_service() {
    // Create UART service over BLE
    uart = new MicroBitUARTService(*uBit.ble, 32, 32);
    uart->eventOn("\n");

    // Register event handlers
    uBit.messageBus.listen(
        MICROBIT_ID_BLE,
        MICROBIT_BLE_EVT_CONNECTED,
        onBLEConnected
    );

    uBit.messageBus.listen(
        MICROBIT_ID_BLE,
        MICROBIT_BLE_EVT_DISCONNECTED,
        onBLEDisconnected
    );

    uBit.messageBus.listen(
        MICROBIT_ID_BLE_UART,
        MICROBIT_UART_S_EVT_DELIM_MATCH,
        onDataReceived
    );
}

int main() {
    uBit.init();

    setup_ble_echo_service();

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

    int scriptLength =
        _binary_source_lua_script_lua_end -
        _binary_source_lua_script_lua_start;

    if (luaL_loadbuffer(L, (const char*)_binary_source_lua_script_lua_start,
                        scriptLength, "embedded") == LUA_OK)
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
