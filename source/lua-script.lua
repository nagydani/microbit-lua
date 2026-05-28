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

uBit.display.scroll("Lua alive")
uBit.audio.setVolume(255)
uBit.audio.express("giggle")
while true do
  uBit.display.animate(heart, 1000, 5)
  uBit.sleep(2000)
  uBit.display.scroll(uBit.friendlyName())
end
