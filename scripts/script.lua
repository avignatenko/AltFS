xpl = require "xplane"

fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}

local yoke_pitch_ratio = xpl.dataref:new("sim/joystick/yoke_pitch_ratio", xpl.types.float, 10, "rw")

offsets=
{
[0x0bb2]={
  type = fsuipc_types.sint16, 
  read = function() return yoke_pitch_ratio:read() * 16383 end,  
  write = function(value)  yoke_pitch_ratio:write(value / 16383 ) end
  }
}


function initialize()
  log(1, "Everything is perfect!!!")

  --xpl.start_autodiscovery()
  xpl.connect("192.168.0.114", 49000)

  return true
end

function shutdown()
  return true
end