local uBit = microbit

local heart = {
  width = 10,
  height = 5,
  data = {
      0,   0,   0,   0,   0,    0, 255,   0, 255,   0,
      0, 255,   0, 255,   0,  255,   0, 255,   0, 255,
      0, 255, 255, 255,   0,  255,   0,   0,   0, 255,
      0,   0, 255,   0,   0,    0, 255,   0, 255,   0,
      0,   0,   0,   0,   0,    0,   0, 255,   0,   0
  }
}

uBit.display.scroll("Lua alive")
while true do
  uBit.display.animateAsync(heart, 1000, 5)
  uBit.sleep(10000)
  uBit.display.scroll(uBit.friendlyName())
end
