extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// only defined in Lua version >=5.2
#define LUA_OK 0

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

static int l_scroll(lua_State *L) {
    uBit.display.scroll(luaL_checkstring(L, 1));
    return 0;
}

/* Register functions into Lua */
static void register_lua_api(lua_State *L) {
    lua_register(L, "scroll", l_scroll);
}


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

    /* Example Lua script controlling LEDs */
    const char *script = R"(
scroll('Lua alive')
while true do
  sleep(5000)
  scroll('tick')
end
)";

    if (luaL_dostring(L, script) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        if (err) {
            uBit.display.scroll("Lua error");
        }
    }

    lua_close(L);

    uBit.display.scroll("Lua exited");

    release_fiber();
}
