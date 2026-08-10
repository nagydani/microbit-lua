nezha2 = {
  turns = 1,
  degrees = 2,
  seconds = 3,
  clockwise = 1,
  counterclockwise = 2,
  shortestarc = 3
}

local char = string.char
local write = microbit.i2c.write

local function send(s)
  write(32, s)
end

local function hl(x)
  local l = x % 256
  local h = (x - l) / 256
  return h, l
end

local function abs(x)
  return math.abs(x), x < 0 and 2 or 1
end

function nezha2.motor_turn(motor, amount, unit)
  local amount_A, direction = abs(amount)
  local amount_H, amount_L = hl(amount_A)
  send("\255\249"..char(motor)..char(direction)..
    "\112"..char(amount_H)..char(unit)..char(amount_L))
end
