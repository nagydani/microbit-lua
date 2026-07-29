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

uBit.audio.setVolume(10)
uBit.audio.express("giggle")
uBit.display.animate(heart, 1000, 5)
uBit.display.scrollAsync(uBit.friendlyName())

local send = uBit.serial.send
local sendAsync = uBit.serial.sendAsync

local function write(s)
  for c in string.gmatch(s, ".") do
    if c == "\n" then
      send("\r")
    end
    send(c)
  end
end

io = {
  write = write
}

local unpack = unpack or table.unpack

local function is_identifier(str)
  return type(str) == "string"
    and str:match("^[%a_][%w_]*$")
end

local prettyprint = { }

local function serialize(value, visited)
  visited = visited or {}
  local pp = prettyprint[type(value)]
  if pp then
    return pp(value, visited)
  end
  return tostring(value)
end

function prettyprint.string(value)
  return string.format("%q", value)
end

local function key(k, visited)
  if is_identifier(k) then
    return k
  else
    return "[" .. serialize(k, visited) .. "]"
  end
end

local function table_tokens(value, visited)
  local out = {"{"}
  local first = true
  for k, v in pairs(value) do
    if not first then
       table.insert(out, ", ")
    end
    first = false
    table.insert(out, key(k, visited) .. " = " .. serialize(v, visited))
  end
  table.insert(out, "}")
  return out
end

function prettyprint.table(value, visited)
  if visited[value] then
    return "<cycle>"
  end
  visited[value] = true
  local out = table_tokens(value, visited)
  visited[value] = nil
  return table.concat(out)
end

local function print_values(serialize, ...)
  local n = select("#", ...)
  if n == 0 then
    return
  end
  local out = { }
  for i = 1, n do
    table.insert(out, serialize(select(i, ...)))
  end
  write(table.concat(out, "\t") .. "\n")
end

local env = { }
setmetatable(env, {
  __index = _G
})

local function load_with_env(code, chunkname)
  local fn, err = loadstring(code, chunkname)
  if fn then
    setfenv(fn, env)
  end
  return fn, err
end

local function is_incomplete(err)
  return err and err:find("<eof>", 1, true) ~= nil
end

local function compile_try(code)
  local chunk, err = load_with_env(code, "REPL")
  if chunk then
    return chunk
  elseif is_incomplete(err) then
    return nil, err, true
  end
  return chunk, err, false
end

local function compile(code)
  local chunk, err, incomp = compile_try(code)
  if chunk or incomp then
    return chunk, err, incomp
  end
  local e_chunk, e_err, e_incomp = compile_try("return " .. code)
  if e_chunk or e_incomp then
    return e_chunk, e_err, e_incomp
  end
  return nil, err or e_err, false
end

function print(...)
  print_values(tostring, ...)  
end

local to_string = tostring

local PRECISION = 1e-6

local function fraction(o, i, f)
  local p = PRECISION
  while f >= p do
    i, f = math.modf(10 * f)
    table.insert(o, string.format("%d", i))
    p = 10 * p
  end
end

local function number2str(n)
  local i, f = math.modf(n)
  local o = {
    string.format("%d", i)
  }
  if f >= PRECISION then
    table.insert(o, ".")
    fraction(o, i, f)
  end
  return table.concat(o)
end

function tostring(o)
  if type(o) ~= "number" then
    return to_string(o)
  end
  return number2str(o)
end

local getChar = uBit.serial.getCharAsync

local buffer = ""

print("micro:bit\nLua 5.1 REPL")

local function prompt()
  write(buffer == "" and "> " or ">> ")
end

local function execute(chunk)
  local results = { pcall(chunk) }
  if results[1] then
    if #results > 1 then
      local out = { }
      for i = 2, #results do
        table.insert(out, serialize(results[i]))
      end
      print("=> " .. table.concat(out, "\t"))
    end
  else
    print("Runtime error: " .. tostring(results[2]))
  end
end

function on_event(source, value, timestamp)
  if source == microbit.DEVICE_ID_SERIAL and value == microbit.CODAL_SERIAL_EVT_HEAD_MATCH then
    local c = getChar()
    while c do
      if c == "\r" then
        buffer = buffer .. "\n"
        write("\r\n")
        local chunk, err, incomplete = compile(buffer)
        if not incomplete then
          if chunk then
            execute(chunk)
          else
            print("Compile error: " .. tostring(err))
          end
          buffer = ""
        end
        prompt()
      elseif c == "\b" or c == "\x7f" then
        if #buffer > 0 then
          buffer = buffer:sub(1, -2)
          write("\b \b")
        end
      else
        buffer = buffer .. c
        write(c)
      end
      c = getChar()
    end
    uBit.serial.eventAfterAsync(1)
    return
  end

  local btn
  if source == microbit.DEVICE_ID_BUTTON_A then
    btn = "A"
  elseif source == microbit.DEVICE_ID_BUTTON_B then
    btn = "B"
  elseif source == microbit.DEVICE_ID_BUTTON_AB then
    btn = "AB"
  end
  if btn then
    if value == microbit.DEVICE_BUTTON_EVT_CLICK then
      uBit.display.scrollAsync(btn)
    elseif value == microbit.DEVICE_BUTTON_EVT_LONG_CLICK then
      uBit.display.scrollAsync(btn .. "!")
    end
  end
end

-- Script-level setup (runs once before the main fiber is released):
-- show prompt, initialise the serial RX buffer, and arm the first per-char
-- head-match event.  After this returns, release_fiber() in main() hands
-- control to the scheduler; on_event() handles all events from the bus.
prompt()
uBit.serial.getCharAsync()
uBit.serial.eventAfterAsync(1)

