local uBit = microbit

uBit.display.scroll("Lua alive")
while true do
  uBit.sleep(5000)
  uBit.display.scroll(uBit.friendlyName())
end
