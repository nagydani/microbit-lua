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
    X(serialNumber, { lua_pushinteger(L, microbit_serial_number());	\
                    return 1;						\
                  })							\
    X(friendlyName, { lua_pushstring(L, microbit_friendly_name());	\
                    return 1;						\
                  })							\
    X(panic,      { int statusCode = (int)luaL_checkinteger(L, 1);      \
                    microbit_panic(statusCode);				\
                    return 0;						\
                  })

// create a Lua image table from C ImageData and place it on the stack
void lua_createimage(lua_State *L, ImageData *ptr) {
  int size = ptr->width * ptr->height;
  lua_createtable(L, 0, 3);
  lua_pushliteral(L, "width");
  lua_pushinteger(L, ptr->width);
  lua_settable(L, -3);
  lua_pushliteral(L, "height");
  lua_pushinteger(L, ptr->height);
  lua_settable(L, -3);
  lua_pushliteral(L, "data");
  lua_createtable(L, size, 0);
  for(int i = 0; i < size; i++) {
    lua_pushinteger(L, ptr->data[i]);
    lua_rawseti(L, -2, i + 1);
  }
  lua_settable(L, -3);
}

// check and return a Lua function argument as a C++ Image object
Image luaL_checkimage(lua_State *L, int narg) {
  Image r;
  int width;
  int height;
  uint8_t value;
  luaL_checktype(L, narg, LUA_TTABLE);
  lua_getfield(L, narg, "width");
  width = (int)lua_tointeger(L, -1);
  lua_getfield(L, narg, "height");
  height = (int)lua_tointeger(L, -1);
  lua_pop(L, 2);
  r = Image(width, height);
  lua_getfield(L, narg, "data");
  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      lua_pushinteger(L, 1 + x + width * y);
      lua_gettable(L, -2);
      value = (uint8_t)lua_tointeger(L, -1);
      lua_pop(L, 1);
      if(r.setPixelValue(x, y, value) != DEVICE_OK) {
        lua_error(L);
      }
    }
  }
  lua_pop(L, 1);
  return r;
}

// see https://rneacy.dev/mbv2/ubit/display/
#define LUA_DISPLAY_FUNCTIONS						\
    X(getWidth,   { lua_pushinteger(L, uBit.display.getWidth());	\
                    return 1;						\
                  })							\
    X(getHeight,  { lua_pushinteger(L, uBit.display.getHeight());	\
                    return 1;						\
                  })							\
    X(setBrightness, { int b = luaL_checkint(L, 1);			\
                    int r = uBit.display.setBrightness(b);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(getBrightness, { int r = uBit.display.getBrightness();		\
                    lua_pushinteger(L, r);				\
                    return 1;						\
                  })							\
    X(enable,     { uBit.display.enable();				\
                    return 0;						\
                  })							\
    X(disable,     { uBit.display.disable();				\
                    return 0;						\
                  })							\
    X(screenShot, { ImageData *ptr =					\
                      uBit.display.screenShot().leakData();		\
                    lua_createimage(L, ptr);				\
                    ptr->decr();					\
                    return 1;						\
                  })							\
    X(setDisplayMode, { DisplayMode mode = 				\
                      static_cast<DisplayMode>(luaL_checkinteger(L, 1));\
                    uBit.display.setDisplayMode(mode);			\
                    return 0;						\
                  })							\
    X(getDisplayMode, { DisplayMode mode =				\
                      uBit.display.getDisplayMode();			\
                    lua_pushinteger(L, static_cast<lua_Integer>(mode));	\
                    return 1;						\
                  })							\
    X(clear,      { uBit.display.clear();				\
                    return 0;						\
                  })							\
    X(readLightLevel, { int r = uBit.display.readLightLevel();		\
                    lua_pushinteger(L, r);				\
                    return 1;						\
                  })							\
    X(setSleep,   { NRF52LEDMatrix display = uBit.display;		\
                    luaL_checkany(L, 1);				\
                    display.setSleep(lua_toboolean(L, 1) != 0);		\
                    return 0;						\
                  })							\
    X(stopAnimation, { uBit.display.stopAnimation();			\
                    return 0;						\
                  })							\
    X(printAsync, { const char *s = luaL_checkstring(L, 1);		\
                    int delay = luaL_optint(L, 2,			\
                      DISPLAY_DEFAULT_PRINT_SPEED);			\
                    int r = uBit.display.printAsync(s, delay);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(print,      { const char *s = luaL_checkstring(L, 1);		\
                    int delay = luaL_optint(L, 2,			\
                      DISPLAY_DEFAULT_PRINT_SPEED);			\
                    int r = uBit.display.print(s, delay);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(scrollAsync, { const char *s = luaL_checkstring(L, 1);		\
                    int delay = luaL_optint(L, 2,			\
                      DISPLAY_DEFAULT_SCROLL_SPEED);			\
                    int r = uBit.display.scrollAsync(s, delay);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(scroll,     { const char *s = luaL_checkstring(L, 1);		\
                    int delay = luaL_optint(L, 2,			\
                      DISPLAY_DEFAULT_SCROLL_SPEED);			\
                    int r = uBit.display.scroll(s, delay);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(animateAsync, { Image image = luaL_checkimage(L, 1);		\
                    int delay = luaL_checkint(L, 2);			\
                    int stride = luaL_checkint(L, 3);			\
                    int startingPosition =				\
                      luaL_optint(L, 4, DISPLAY_ANIMATE_DEFAULT_POS);	\
                    int autoClear =					\
                      luaL_optint(L, 5, DISPLAY_DEFAULT_AUTOCLEAR);	\
                    int r = uBit.display.animateAsync(image,		\
                                                      delay,		\
                                                      stride,		\
                                                      startingPosition,	\
                                                      autoClear);	\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(animate,    { Image image = luaL_checkimage(L, 1);		\
                    int delay = luaL_checkint(L, 2);			\
                    int stride = luaL_checkint(L, 3);			\
                    int startingPosition =				\
                      luaL_optint(L, 4, DISPLAY_ANIMATE_DEFAULT_POS);	\
                    int autoClear =					\
                      luaL_optint(L, 5, DISPLAY_DEFAULT_AUTOCLEAR);	\
                    int r = uBit.display.animate(image,			\
                                                 delay,			\
                                                 stride,		\
                                                 startingPosition,	\
                                                 autoClear);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(setPixelValue, {							\
                    uint16_t x = (uint16_t)luaL_checkint(L, 1);		\
                    uint16_t y = (uint16_t)luaL_checkint(L, 2);		\
                    uint8_t value = (uint8_t)luaL_checkint(L, 3);	\
                    int r =						\
                      uBit.display.image.setPixelValue(x, y, value);	\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(getPixelValue, {							\
                    uint16_t x = (uint16_t)luaL_checkint(L, 1);		\
                    uint16_t y = (uint16_t)luaL_checkint(L, 2);		\
                    int r = uBit.display.image.getPixelValue(x, y);	\
                    if(r != DEVICE_INVALID_PARAMETER) {			\
                      lua_pushinteger(L, r);				\
                      return 1;						\
                    }							\
                    return 0;						\
                  })

// I failed counting X expansions in LUA_DISPLAY_FUNCTIONS automatically
#define LUA_DISPLAY_COUNT 21

// see https://rneacy.dev/mbv2/ubit/accelerometer/
#define LUA_ACCELEROMETER_FUNCTIONS					\
    X(setPeriod,  { int period = luaL_checkint(L, 1);			\
                    int r = uBit.accelerometer.setPeriod(period);	\
                    lua_pushboolean(L, r == MICROBIT_OK);		\
                    return 1;						\
                  })							\
    X(getPeriod,  { int r = uBit.accelerometer.getPeriod();		\
                    lua_pushinteger(L, r);				\
                    return 1;						\
                  })							\
    X(setRange,   { int range = luaL_checkint(L, 1);			\
                    int r = uBit.accelerometer.setRange(range);		\
                    lua_pushboolean(L, r == MICROBIT_OK);		\
                    return 1;						\
                  })							\
    X(getRange,   { int r = uBit.accelerometer.getRange();		\
                    lua_pushinteger(L, r);				\
                    return 1;						\
                  })							\
    X(getXYZ,     { lua_pushinteger(L, uBit.accelerometer.getX());	\
                    lua_pushinteger(L, uBit.accelerometer.getY());	\
                    lua_pushinteger(L, uBit.accelerometer.getZ());	\
                    return 3;						\
                  })							\
    X(getGesture, { uint16_t r = uBit.accelerometer.getGesture();	\
                    lua_pushinteger(L, r);				\
                    return 1;						\
                  })

#define LUA_ACCELEROMETER_COUNT 6

#define LUA_AUDIO_FUNCTIONS						\
    X(getPin,     { lua_pushlightuserdata(L,				\
                      &uBit.audio.virtualOutputPin);			\
                    return 1;						\
                  })							\
    X(setVolume,  { int volume = luaL_checkint(L, 1);			\
                    int r = uBit.audio.setVolume(volume);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(getVolume,  { lua_pushinteger(L, uBit.audio.getVolume());		\
                    return 1;						\
                  })							\
    X(express,    { const char *expression = luaL_checkstring(L, 1);	\
                    uBit.audio.soundExpressions.playAsync(expression);	\
                    return 0;						\
                  })

#define LUA_AUDIO_COUNT 4

Pin *luaL_checkPin(lua_State *L, int narg) {
  luaL_checktype(L, narg, LUA_TLIGHTUSERDATA);
  return (Pin *)lua_touserdata(L, narg);
}

#define LUA_IO_FUNCTIONS						\
    X(setDigitalValue, { 						\
                    Pin *pin = luaL_checkPin(L, 1);			\
                    int value = luaL_checkint(L, 2);			\
                    int r = pin->setDigitalValue(value);		\
                    lua_pushboolean(L, r == DEVICE_OK);			\
                    return 1;						\
                  })							\
    X(getDigitalValue, {						\
                    Pin *pin = luaL_checkPin(L, 1);			\
                    int r = pin->getDigitalValue();			\
                    if(r == 0 || r == 1) {				\
                      lua_pushinteger(L, r);				\
                    } else {						\
                      lua_pushnil(L);					\
                    }							\
                    return 1;						\
                  })							\


#define LUA_IO_COUNT 2

#define X(name, body) static int l_##name(lua_State *L) body
LUA_MICROBIT_FUNCTIONS
LUA_DISPLAY_FUNCTIONS
LUA_ACCELEROMETER_FUNCTIONS
LUA_AUDIO_FUNCTIONS
LUA_IO_FUNCTIONS
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
static const luaL_Reg l_accelerometer[] = {
    LUA_ACCELEROMETER_FUNCTIONS
    {NULL, NULL}
};
static const luaL_Reg l_audio[] = {
    LUA_AUDIO_FUNCTIONS
    {NULL, NULL}
};
static const luaL_Reg l_io[] = {
    LUA_IO_FUNCTIONS
    {NULL, NULL}
};
#undef X

void register_lua_api(lua_State *L) {
  luaL_register(L, "microbit", l_microbit);
  lua_createtable(L, 0, LUA_DISPLAY_COUNT);
  luaL_register(L, NULL, l_display);
  lua_setfield(L, -2, "display");
  lua_createtable(L, 0, LUA_ACCELEROMETER_COUNT);
  luaL_register(L, NULL, l_accelerometer);
  lua_setfield(L, -2, "accelerometer");
  lua_createtable(L, 0, LUA_AUDIO_COUNT);
  luaL_register(L, NULL, l_audio);
  lua_setfield(L, -2, "audio");
  luaL_register(L, NULL, l_io);
  lua_setfield(L, -2, "io");
}
