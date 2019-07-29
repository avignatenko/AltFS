
fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}


function connected()

	local yoke_pitch_ratio = xplane.dataref:new("sim/joystick/yoke_pitch_ratio", xplane.types.float, 1)
	local yoke_roll_ratio = xplane.dataref:new("sim/joystick/yoke_roll_ratio", xplane.types.float, 1)
	local sim_paused = xplane.dataref:new("sim/time/paused", xplane.types.int, 5)
	local ap_master = xplane.dataref:new("sim/cockpit/autopilot/autopilot_mode", xplane.types.int, 2) -- fixme, cockpit2?
    local altitude_hold_status = xplane.dataref:new("sim/cockpit2/autopilot/altitude_hold_status", xplane.types.int, 2)
    local elevator_trim = xplane.dataref:new("sim/flightmodel2/controls/elevator_trim", xplane.types.float, 30)
	
	
	offsets=
	{
	-- Custom BFF Offset 
	[0x0588]={ fsuipc_types.float64, function() return local_time_sec end, function(value) log(1, "error: can't write into readonly var") end },
	
	-- Pause indicator (0=Not paused, 1=Paused)
	[0x0264]={ fsuipc_types.uint16, function() return sim_paused:read() > 0 and 1 or 0 end, function(value) log(1, "error: can't write into readonly var") end },
	[0x0bb2]={ fsuipc_types.sint16, function() return yoke_pitch_ratio:read() * 16383 end, function(value)  yoke_pitch_ratio:write(value / 16383 ) end },
	[0x0bb6]={ fsuipc_types.sint16, function() return yoke_roll_ratio:read() * 16383 end, function(value)  yoke_roll_ratio:write(value / 16383 ) end },
	
	-- Slew mode (indicator and control), 0=off, 1=on. (See 05DE also).
	[0x05dc]={ fsuipc_types.uint16, function() return 0 end, function(value) end },
	-- Autopilot Master switch
	[0x07bc]={ fsuipc_types.uint32, function() return ap_master:read() == 2 and 1 or 0 end, function(value) ap_master:write(value and 2 or 0) end },
	-- Autopilot altitude lock
	[0x07d0]={ fsuipc_types.uint32, function() return altitude_hold_status:read() ~= 0 and 1 or 0 end, function(value) log(1, "error: can't write into readonly var") end },
	-- Crashed flag
	[0x0840]={ fsuipc_types.sint16, function() return 0 end, function(value) end },
	-- Elevator trim control input: –16383 to +16383
	[0x0bc0]={ fsuipc_types.sint16,  function() return elevator_trim:read() * 16383 end, function(value)  elevator_trim:write(value / 16383 ) end },
	-- Ready to Fly indicator. This is non-zero when FS is loading, or reloading a flight or aircraft or scenery, and 
	-- becomes zero when flight mode is enabled (even if the simulator is paused or in Slew mode).
	[0x3364]={ fsuipc_types.uint8, function() return sim_paused:read() and 0 or 1 end, function(value) log(1, "error: can't write into readonly var")  end },
	-- In Menu or Dialog flag. 
	[0x3365]={ fsuipc_types.uint8, function() return sim_paused:read() end, function(value) log(1, "error: can't write into readonly var")  end },

	}

end

function initialize()
  log(1, "Everything is perfect!!!")

  --xplane.start_autodiscovery()
  xplane.connect("192.168.0.114", 49000)

  return true
end

function shutdown()
  return true
end