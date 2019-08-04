
fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}


local yoke_pitch_ratio = xplane.dataref:new("sim/joystick/yoke_pitch_ratio", xplane.types.float, 20)
local yoke_roll_ratio = xplane.dataref:new("sim/joystick/yoke_roll_ratio", xplane.types.float, 20)
local sim_paused = xplane.dataref:new("sim/time/paused", xplane.types.int, 5)
local ap_master = xplane.dataref:new("sim/cockpit/autopilot/autopilot_mode", xplane.types.int, 2) -- fixme, cockpit2?
local altitude_hold_status = xplane.dataref:new("sim/cockpit2/autopilot/altitude_hold_status", xplane.types.int, 2)
local elevator_trim = xplane.dataref:new("sim/flightmodel2/controls/elevator_trim", xplane.types.float, 30)
local local_time_sec = xplane.dataref:new("sim/time/local_time_sec", xplane.types.float, 30)
	
	
offsets=
{

-- IN PROGRESS {

-- Minute of local time in FS (0–59)
[0x0239] = { fsuipc_types.uint8, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Second of time in FS (0–59)
[0x023a] = { fsuipc_types.uint8, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },

-- GS: Ground Speed, as 65536*metres/sec. Not updated in Slew mode!
[0x02b4] = { fsuipc_types.uint32, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },


--# Custom offset - 0x66E0 CG Position Displacement from default in meters  (changed from previous 6700)
--Dataref	BFF_CG_Pos_Disp	sim/flightmodel/misc/cgz_ref_to_default	float
--Offset	0x66E0  FLOAT64	1	r	$BFF_CG_Pos_Disp
[0x66e0] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },

--# Custom offset - 0x66E8 - stall warning Angle of Attack - in Degrees
--Dataref BFF_AOA_Warn	sim/aircraft/overflow/acf_stall_warn_alpha	float
--Offset	0x66E8	FLOAT64	1	r	$BFF_AOA_Warn
[0x66e8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },

--# Custom offset - 0x66F8 Engine 1 wash in m/s
--Dataref	BFF_PWash	sim/flightmodel/jetwash/DVinc[0]	floatarray
--Offset	0x66F8  FLOAT64	1	r	$BFF_PWash
[0x66f8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },

--TAS: True Air Speed, as knots * 128	
[0x02b8] = { fsuipc_types.uint32, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Aircraft on ground flag (0=airborne, 1=on ground). Not updated in Slew mode
--# Substitution offset - 0x0366 - On-ground indicator - hardcoded offset is not sensitive enough for vibration detection
--Dataref BFF_Gear_F1 sim/flightmodel/forces/fside_gear float
--Dataref BFF_Gear_F2 sim/flightmodel/forces/fnrml_gear float
--Dataref BFF_Gear_F3 sim/flightmodel/forces/faxil_gear float
--Offset  0x0366 UINT16 1 r $BFF_Gear_F1 $BFF_Gear_F2 + $BFF_Gear_F3 + 0 >
[0x0366] = { fsuipc_types.uint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Stall warning (0=no, 1=stall)
[0x036c] = { fsuipc_types.uint8, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Engine 1 Jet N1 as 0 – 16384 (100%), or Prop RPM (derive RPM by multiplying this value by the RPM Scaler (see 08C8) and dividing by 65536). Note that Prop RPM is signed and negative for counter-rotating propellers.
[0x0898] = { fsuipc_types.uint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Engine 1 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
--# Substitute offset - to zero the fuel flow when engine 1 is not running
--# Does this by multiplying the fuel flow parameter by the engine running parameter (0 or 1)
--Dataref	BFF_Fuel_Flow_1	sim/flightmodel/engine/ENGN_FF_[0]		floatarray
--Dataref	BFF_Eng1_Running	sim/flightmodel/engine/ENGN_running[0]		intarray
--Offset 0x0918 FLOAT64 1 r $BFF_Eng1_Running $BFF_Fuel_Flow_1 * 7936.64 *
[0x0918] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Engine 2 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
[0x09b0] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Fail mode, 0 ok, Hydraulics failure = 1
[0x0b62] = { fsuipc_types.uint8, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Rudder control input: –16383 to +16383
[0x0bba] = { fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--Gear position (right): 0=full up, 16383=full down
[0x0bf0] = { fsuipc_types.uint32, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Aileron trim value/control: –16383 to +16383
--# Custom offset - 0x0C02 Set Aileron Trim - data ref range +/1.0 scaled to +/- 16383
--Dataref	BFF_ATrim	sim/flightmodel/controls/ail_trim	float
--Offset	0x0C02 SINT16 1 rw	$BFF_ATrim 16383 * >BFF_ATrim @ 16383 /
[0x0c02] = { fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Rudder trim value/control: –16383 to +16383
--# Custom offset - 0x0C04 Set Rudder Trim - data ref range +/1.0 scaled to +/- 16383
--Dataref	BFF_RTrim	sim/flightmodel/controls/rud_trim	float
--Offset	0x0C04 SINT16 1 rw	$BFF_RTrim 16383 * >BFF_RTrim @ 16383 /
[0x0c04] = { fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
--G Force: units unknown, but /624 seems to give quite sensible values. See also offset 1140
[0x11ba] = { fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Angle of Attack Indicator angle, with 360 degrees = 65536. The value 32767 is 180 degrees Angle of Attack. The angle is expressed in the usual FS 16-bit angle units (360 degrees = 65536), with 180 degrees pointing to the 0.0 position (right and down about 35 degrees in a Boeing type AofA indicator). Note that the indicator angle actually decreases as the wing AofA increases.
[0x11be] = { fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Turbine Engine 1 jet thrust, in pounds, as a double (FLOAT64). This is the jet thrust. See 2410 for propeller thrust (turboprops have both)
[0x204c] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Propeller 1 thrust in pounds, as a double (FLOAT64). This is for props and turboprops.
--# FSUIPC offset - 0x2410 Engine 1 thrust in Pounds (comes in as Newtons)
--#Dataref	BFF_Thrust	sim/flightmodel/engine/POINT_thrust[0] floatarray
--#Offset	0x2410  FLOAT64	1	r	$BFF_Thrust 0.2243 *
[0x2410] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Aileron deflection, in radians, as a double (FLOAT64). Right turn positive, left turn negative. (This is the average of left and right)
[0x2ea8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Incidence “alpha”, in radians, as a double (FLOAT64). This is the aircraft body angle of attack (AoA) not the wing AoA.
[0x2ed0] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Incidence “beta”, in radians, as a double (FLOAT64). This is the side slip angle.
[0x2ed8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- CG percent, as a double (FLOAT64). This is the position of the actual CoG as a fraction (%/100) of MAC (Mean Aerodynamic Chord).
[0x2ef8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- X (lateral, or left/right) acceleration in ft/sec/sec relative to the body axes in double floating point format.
[0x3060] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Roll acceleration in radians/sec/sec relative to the body in double floating point format.
[0x3080] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
-- Pitch velocity in rads/sec relative to the body axes in double floating point format.
[0x30a8] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },

--# Created offset - 0x6030 Aircraft ground speed, double, in m/s.
--# sim/flightmodel/position/groundspeed	float	n	meters/sec	The ground speed of the aircraft
--Dataref	BFF_Grnd_Speed	sim/flightmodel/position/groundspeed	float
--Offset	0x6030 FLOAT64 1 rw	$BFF_Grnd_Speed
[0x6030] = { fsuipc_types.float64, function() return 0 end, function(value) log(loglevel.error, "error: can't write into readonly var") end },


-- }

-- Custom BFF Offset 
[0x0588]={ fsuipc_types.float64, function() return local_time_sec:read() end, function(value) log(loglevel.error, "error: can't write into readonly var") end },
	
-- Pause indicator (0=Not paused, 1=Paused)
[0x0264]={ fsuipc_types.uint16, function() return sim_paused:read() > 0 and 1 or 0 end, function(value) log(loglevel.error, "can't write into readonly var") end },
-- Pitch
[0x0bb2]={ fsuipc_types.sint16, function() return yoke_pitch_ratio:read() * 16383 end, function(value) yoke_pitch_ratio:write(value / 16383 ) end },
-- Roll
[0x0bb6]={ fsuipc_types.sint16, function() return yoke_roll_ratio:read() * 16383 end, function(value)  yoke_roll_ratio:write(value / 16383 ) end },
	
-- Slew mode (indicator and control), 0=off, 1=on. (See 05DE also).
[0x05dc]={ fsuipc_types.uint16, function() return 0 end, function(value) end },
-- Autopilot Master switch
[0x07bc]={ fsuipc_types.uint32, function() return ap_master:read() == 2 and 1 or 0 end, function(value) ap_master:write(value and 2 or 0) end },
-- Autopilot altitude lock
[0x07d0]={ fsuipc_types.uint32, function() return altitude_hold_status:read() ~= 0 and 1 or 0 end, function(value) log(loglevel.error, "can't write into readonly var") end },
-- Crashed flag
[0x0840]={ fsuipc_types.sint16, function() return 0 end, function(value) log(loglevel.error, "can't write into readonly var") end },
-- Elevator trim control input: –16383 to +16383
[0x0bc0]={ fsuipc_types.sint16,  function() return elevator_trim:read() * 16383 end, function(value)  elevator_trim:write(value / 16383 ) end },
-- Ready to Fly indicator. This is non-zero when FS is loading, or reloading a flight or aircraft or scenery, and 
-- becomes zero when flight mode is enabled (even if the simulator is paused or in Slew mode).
[0x3364]={ fsuipc_types.uint8, function() return sim_paused:read() and 0 or 1 end, function(value) log(1, "error: can't write into readonly var")  end },
-- In Menu or Dialog flag. 
[0x3365]={ fsuipc_types.uint8, function() return sim_paused:read() end, function(value) log(1, "error: can't write into readonly var")  end },

}
