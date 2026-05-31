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

uBit.audio.setVolume(255)
uBit.audio.express("giggle")
uBit.display.animate(heart, 1000, 5)
uBit.display.scrollAsync(uBit.friendlyName())

local send = uBit.serial.send

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

local unpack = table.unpack

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

local getChar = uBit.serial.getChar

local function readLine()
  local out = { }
  local b = getChar()
  while b ~= "\r" do
    table.insert(out, b)
    write(b)
    b = getChar()
  end
  write("\n")
  return table.concat(out)
end

local buffer = ""

print("micro:bit\nLua 5.1 REPL")

while true do
  write(buffer == "" and "> " or ">> ")
  local line = readLine()
  buffer = buffer .. line .. "\n"
  local chunk, err, incomplete = compile(buffer)
  if incomplete then
    -- Continue collecting lines
  elseif not chunk then
    print("Compile error: " .. tostring(err))
    buffer = ""
  else
    local results = { pcall(chunk) }
    local ok = results[1]
    if ok then
      if #results > 1 then
        print_values(serialize, unpack(results, 2))
      end
    else
      print("Runtime error: " .. tostring(results[2]))
    end
    buffer = ""
  end
end

