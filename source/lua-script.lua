local uBit = microbit

local heart = {
  width = 10,
  height = 5,
  data = {
      0,   0,   0,   0,   0,    0, 255,   0, 255,   0,
      0, 255,   0, 255,   0,  255,  64, 255,  64, 255,
      0, 255, 255, 255,   0,  255,  64,  64,  64, 255,
      0,   0, 255,   0,   0,    0, 255,  64, 255,   0,
      0,   0,   0,   0,   0,    0,   0, 255,   0,   0
  }
}

local friendlyName = uBit.friendlyName()

uBit.audio.setVolume(255)
uBit.audio.express("giggle")
uBit.display.animate(heart, 1000, 5)
uBit.serial.send("micro:bit Lua\n")
while true do
  uBit.sleep(5000)
  uBit.display.scroll(friendlyName)
end
