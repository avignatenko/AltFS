xpl = require "xplane"

fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}

local testVar = xpl.dataref:new("sim/some/variable", xpl.types.float, 30, "rw")
local testVar2 = xpl.dataref:new("sim/some/variable2", xpl.types.int, 1, "r")
local testVar3 = xpl.dataref:new("sim/some/variable3", xpl.types.int, 1, "r")

offsets=
{
[0x238]={
	type = fsuipc_types.array,
	size = 3,
	read = function() return string.char(testVar2:read(), testVar3:read(), string.byte("A")) end,
	write = function(value) end
},
[0x225]={
  type = fsuipc_types.uint16, 
  read = function() return testVar:read() * 32768 end,  
  write = function(value)  testVar:write(value / 32768 ) end
  },
  [0x226]={
  type = fsuipc_types.uint8, 
  read = function() end,  
  write = function(value) end
  }
}


function initialize()
  log(1, "Everything is perfect!!!")

 
  return true
end

function shutdown()
  return true
end