tpbot = { }

local char = string.char
local write = microbit.i2c.write

local function send(s)
  write(32, s)
end

function tpbot.set_car_light(r, g, b)
  send("\032"..char(r)..char(g)..char(b))
end

local function abs(x, n)
  return math.abs(x), x < 0 and n or 0
end

function tpbot.set_motors_speed(left, right)
  local l, d = abs(left, 1)
  local r, e = abs(right, 2)
  send("\001"..char(l)..char(r)..char(d + e))
end
