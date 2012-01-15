#!/usr/bin/env luajit

require 'struct'
require 'pl'

NCPU = #dir.getfiles('.', 'crewinc-?')

INCLOG_N = 3

--------------------------------
-- Find out the memory operation
--------------------------------

function get_memop_txt(line)
  local i = line:find(' ', 1, true)
  return tonumber(line:sub(1, i))
end

function get_memop_binary(line)
  return struct.unpack('I4', line)
end

--[[
Each crew inc log has (in same order as C source code)
- self memop:  uint32_t
- owner memop: uint32_t
- owner:       int16_t
--]]

-- Insert to correct place
function insert_back_onebuf(memop, line, buf)
  for i=#buf,1,-1 do
    if memop >= get_memop_txt(buf[i]) then
      table.insert(buf, i, line)
      return true
    end
  end
  return false
end

function insert_back(memop, line, curbuf, prebuf)
  print(string.format("inserting back memop %d", memop))
  if not insert_back_onebuf(memop, line, curbuf) then
    if #prebuf == 0 then
      table.insert(curbuf, 1, line)
    else
      if not insert_back_onebuf(memop, line, prebuf) then
        print("Maybe need to increase bound size")
        os.exit(1)
      end
    end
  end
end

-- Assume the displacement does not exceed 100
local bound = 409
function sort_log(infile, outfile)
  print(string.format("sorting %s", infile))

  local inf = assert(io.open(infile))
  local outf = assert(io.open(outfile, "w"))

  -- First fill buf1, if exceeds bound, switch to another buf
  local curbuf = {}
  local prebuf = {}
  local preop = -1

  while true do
    --local line = inf:read(10)
    local line = inf:read("*line")
    if not line then break end

    --local memop = get_memop_binary(line)
    local memop = get_memop_txt(line)

    -- If no in correct order, insert into the correct place
    if memop < preop then
      insert_back(memop, line, curbuf, prebuf)
    else
      table.insert(curbuf, line)
    end

    if #curbuf > bound then
      -- Write out old buffer first
      outf:write(table.concat(prebuf, '\n'))
      outf:write('\n')
      prebuf = curbuf
      curbuf = {}
    end

    if memop > preop then
      preop = memop
    end
  end

  -- Don't forget to write out what's left in curbuf and prebuf
  outf:write(table.concat(prebuf, '\n'))
  outf:write('\n')
  outf:write(table.concat(curbuf, '\n'))

  inf:close()
  outf:close()
end

for i = 1, NCPU do
  local incfile = string.format('crewinc-%d', i - 1)
  local sortedfile = incfile..'.sorted'
  sort_log(incfile, sortedfile)
  os.rename(sortedfile, incfile)
end

