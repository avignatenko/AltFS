
fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}


function connected()

	local yoke_pitch_ratio = xplane.dataref:new("sim/joystick/yoke_pitch_ratio", xplane.types.float, 1)
	local yoke_roll_ratio = xplane.dataref:new("sim/joystick/yoke_roll_ratio", xplane.types.float, 1)

	offsets=
	{
	[0x0bb2]={
	  type = fsuipc_types.sint16, 
	  read = function() return yoke_pitch_ratio:read() * 16383 end,  
	  write = function(value)  yoke_pitch_ratio:write(value / 16383 ) end
	  },
	[0x0bb6]={
	  type = fsuipc_types.sint16, 
	  read = function() return yoke_roll_ratio:read() * 16383 end,  
	  write = function(value)  yoke_roll_ratio:write(value / 16383 ) end
	  },
	}

end

function initialize()
  log(1, "Everything is perfect!!!")

  xplane.start_autodiscovery()
  --xplane.connect("192.168.0.114", 49000)

  return true
end

function shutdown()
  return true
end