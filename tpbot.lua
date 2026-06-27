tpbot = { }

local char = string.char
local write = microbit.i2c.write

local function send(s)
  write(32, s)
end

function tpbot.set_car_light(r, g, b)
  send("\032"..char(r)..char(g)..char(b))
end
