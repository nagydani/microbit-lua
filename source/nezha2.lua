nezha2 = {
  TURNS = 1,
  DEGREES = 2,
  SECONDS = 3,
  SHORTESTARC = 1,
  CLOCKWISE = 2,
  COUNTERCLOCKWISE = 3
}

local char = string.char
local write = microbit.i2c.write
local read = microbit.i2c.read
local sleep = microbit.sleep

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

function nezha2.motor_goto(motor, mode, angle)
   local angle_H, angle_L = hl(angle % 360)
   send("\255\249"..char(motor).."\000\093"..
     char(angle_H)..char(mode)..char(angle_L))
end

function nezha2.motor_reset(motor)
  send("\255\249"..char(motor).."\000\029\000\245\000")
end

function nezha2.motor_spin(motor, speed)
  local speed_A, direction = abs(speed)
  send("\255\249"..char(motor)..char(direction).."\096"..
    char(speed_A).."\245\000")
end

function nezha2.motor_position(motor)
  send("\255\249"..char(motor).."\000\070\000\245\000")
  sleep(4)
  local p0, p1, p2, p3 = string.byte(read(32, 4), 1, 4)
  return ((((p3 * 256 + p2) * 256 + p1) * 256 + p0) %
    3600) * 0.1
end

function nezha2.motor_speed(motor)
  send("\255\249"..char(motor).."\000\071\000\245\000")
  sleep(3)
  local s0, s1 = string.byte(read(32, 2), 1, 2)
  return (s1 * 256 + s0) * 0.0926
end
