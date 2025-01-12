

fsuipc_types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10, array=11}

local freq = {once = 1, veryhigh = 60, high = 30, medium = 15, low = 5, verylow = 1}

local yoke_pitch_ratio = xplane.dataref:new("sim/joystick/yoke_pitch_ratio", xplane.types.float, freq.high)
local yoke_roll_ratio = xplane.dataref:new("sim/joystick/yoke_roll_ratio", xplane.types.float, freq.high)
local yoke_heading_ratio = xplane.dataref:new("sim/joystick/yoke_heading_ratio", xplane.types.float, freq.high)
local sim_paused = xplane.dataref:new("sim/time/paused", xplane.types.int, freq.low)
local ap_master = xplane.dataref:new("sim/cockpit/autopilot/autopilot_mode", xplane.types.int, freq.low) -- fixme, cockpit2?
local altitude_hold_status = xplane.dataref:new("sim/cockpit2/autopilot/altitude_hold_status", xplane.types.int, freq.verylow)
local elevator_trim = xplane.dataref:new("sim/flightmodel2/controls/elevator_trim", xplane.types.float, freq.high)
local aileron_trim =  xplane.dataref:new("sim/flightmodel2/position/aileron_trim", xplane.types.float, freq.high)
local rudder_trim =  xplane.dataref:new("sim/flightmodel2/position/rudder_trim", xplane.types.float, freq.high)
local local_time_sec = xplane.dataref:new("sim/time/local_time_sec", xplane.types.float, freq.high)
local ground_speed = xplane.dataref:new("sim/flightmodel/position/groundspeed", xplane.types.float, freq.low) --fixme, flightmodel2?
local vertical_speed = xplane.dataref:new("sim/flightmodel/position/vh_ind_fpm2", xplane.types.float, freq.low) --fixme, flightmodel2?
local elevation = xplane.dataref:new("sim/flightmodel/position/elevation", xplane.types.float, freq.low) --fixme, flightmodel2?
local theta = xplane.dataref:new("sim/flightmodel/position/theta", xplane.types.float, freq.low) --fixme, flightmodel2?
local local_time_minutes = xplane.dataref:new("sim/cockpit2/clock_timer/local_time_minutes", xplane.types.int, freq.verylow)
local local_time_seconds = xplane.dataref:new("sim/cockpit2/clock_timer/local_time_seconds", xplane.types.int, freq.verylow)
local cgz_ref_to_default = xplane.dataref:new("sim/flightmodel/misc/cgz_ref_to_default", xplane.types.float, freq.verylow)
local acf_stall_warn_alpha = xplane.dataref:new("sim/aircraft/overflow/acf_stall_warn_alpha", xplane.types.float, freq.once)
local dvinc_0 = xplane.dataref:new("sim/flightmodel2/engines/propwash_mtr_sec[0]", xplane.types.float, freq.medium)
local gearF1 = xplane.dataref:new("sim/flightmodel/forces/fside_gear", xplane.types.float, freq.medium)
local gearF2 = xplane.dataref:new("sim/flightmodel/forces/fnrml_gear", xplane.types.float, freq.medium)
local gearF3 = xplane.dataref:new("sim/flightmodel/forces/faxil_gear", xplane.types.float, freq.medium)
local fuel_flow_1 = xplane.dataref:new("sim/flightmodel/engine/ENGN_FF_[0]", xplane.types.float, freq.low)
local fuel_flow_2 = xplane.dataref:new("sim/flightmodel/engine/ENGN_FF_[1]", xplane.types.float, freq.low)
local fuel_flow_3 = xplane.dataref:new("sim/flightmodel/engine/ENGN_FF_[2]", xplane.types.float, freq.low)
local fuel_flow_4 = xplane.dataref:new("sim/flightmodel/engine/ENGN_FF_[3]", xplane.types.float, freq.low)
local eng1_running = xplane.dataref:new("sim/flightmodel/engine/ENGN_running[0]", xplane.types.float, freq.low)
local eng2_running = xplane.dataref:new("sim/flightmodel/engine/ENGN_running[1]", xplane.types.float, freq.low)
local eng3_running = xplane.dataref:new("sim/flightmodel/engine/ENGN_running[2]", xplane.types.float, freq.low)
local eng4_running = xplane.dataref:new("sim/flightmodel/engine/ENGN_running[3]", xplane.types.float, freq.low)
local true_airspeed = xplane.dataref:new("sim/flightmodel/position/true_airspeed", xplane.types.float, freq.low)
local point_thrust = xplane.dataref:new("sim/flightmodel/engine/POINT_thrust[0]", xplane.types.float, freq.low)
local stall_warning = xplane.dataref:new("sim/cockpit2/annunciators/stall_warning", xplane.types.int, freq.low)
local alpha = xplane.dataref:new("sim/flightmodel/position/alpha", xplane.types.float, freq.high)
local beta = xplane.dataref:new("sim/flightmodel/position/beta", xplane.types.float, freq.high)
local crashed = xplane.dataref:new("sim/flightmodel2/misc/has_crashed", xplane.types.int, freq.low)
local aoa_degrees = xplane.dataref:new("sim/flightmodel2/misc/AoA_angle_degrees", xplane.types.float, freq.high) -- Positive means aircracft nose is above the flight path in aircraft coordinates.
local nose_gear_deploy_ratio = xplane.dataref:new("sim/flightmodel2/gear/deploy_ratio[0]", xplane.types.float, freq.medium)
local right_gear_deploy_ratio = xplane.dataref:new("sim/flightmodel2/gear/deploy_ratio[1]", xplane.types.float, freq.medium)
local left_gear_deploy_ratio = xplane.dataref:new("sim/flightmodel2/gear/deploy_ratio[2]", xplane.types.float, freq.medium)
local hydraulic_pressure_low = xplane.dataref:new("sim/cockpit2/annunciators/hydraulic_pressure", xplane.types.int, freq.verylow)
local g_nrml = xplane.dataref:new("sim/flightmodel/forces/g_nrml", xplane.types.float, freq.high)
local g_side = xplane.dataref:new("sim/flightmodel/forces/g_side", xplane.types.float, freq.high)
local g_forw = xplane.dataref:new("sim/flightmodel/forces/g_axil", xplane.types.float, freq.high)
local Qrad = xplane.dataref:new("sim/flightmodel/position/Qrad", xplane.types.float, freq.medium)
local P_dot = xplane.dataref:new("sim/flightmodel/position/P_dot", xplane.types.float, freq.high)
local engine_rpm_0 = xplane.dataref:new("sim/cockpit2/engine/indicators/engine_speed_rpm[0]", xplane.types.float, freq.medium)
local engine_rpm_1 = xplane.dataref:new("sim/cockpit2/engine/indicators/engine_speed_rpm[1]", xplane.types.float, freq.medium)
local engine_rpm_2 = xplane.dataref:new("sim/cockpit2/engine/indicators/engine_speed_rpm[2]", xplane.types.float, freq.medium)
local engine_rpm_3 = xplane.dataref:new("sim/cockpit2/engine/indicators/engine_speed_rpm[3]", xplane.types.float, freq.medium)
local lail1def = xplane.dataref:new("sim/flightmodel/controls/lail1def", xplane.types.float, freq.high)
local ail1def_dn_max = xplane.dataref:new("sim/aircraft/controls/acf_ail1_dn", xplane.types.float, freq.verylow)
local rho = xplane.dataref:new("sim/weather/rho", xplane.types.float, freq.verylow)
local barometer_current_inhg = xplane.dataref:new("sim/weather/barometer_current_inhg", xplane.types.float, freq.verylow)
local surface_texture_type = xplane.dataref:new("sim/flightmodel/ground/surface_texture_type", xplane.types.int, freq.verylow)
local turbulence_percent = xplane.dataref:new("sim/weather/wind_turbulence_percent", xplane.types.int, freq.verylow)
local flapss_left_deployment_degrees = xplane.dataref:new("sim/flightmodel2/wing/flap1_deg[0]", xplane.types.float, freq.low)
local flaps_right_deployment_degrees = xplane.dataref:new("sim/flightmodel2/wing/flap1_deg[1]", xplane.types.float, freq.low)
local flaps_deployment_ratio = xplane.dataref:new("sim/flightmodel2/controls/flap1_deploy_ratio", xplane.types.float, freq.low)
local custom_cl_engage = xplane.dataref:new("ai/cockpit/devices/cl_engage", xplane.types.int, freq.verylow)

local readonly = function(value) log(loglevel.err, "error: can't write into readonly var") end

local cl_trim = 0

offsets=
{

--Supress warning
[0x0] = { fsuipc_types.uint8, function() return 0 end, readonly },

-- version, etc.
[0x3304] = { fsuipc_types.uint32, function() return 0xFFFFFFFF end, readonly },
[0x3308] = { fsuipc_types.uint32, function() return 0xFADE0000 end, readonly },

-- Turbine Engine 1 jet thrust, in pounds, as a double (FLOAT64). This is the jet thrust. See 2410 for propeller thrust (turboprops have both)
[0x204c] = { fsuipc_types.float64, function() return 0 end, readonly },
-- CG percent, as a double (FLOAT64). This is the position of the actual CoG as a fraction (%/100) of MAC (Mean Aerodynamic Chord).
[0x2ef8] = { fsuipc_types.float64, function() return 0 end, readonly },

-- Custom BFF Offset

[0x66C0] = { fsuipc_types.uint8, function() return ap_master:read() == 2 and 1 or 0 end, readonly },
[0x66C1] = { fsuipc_types.uint8, function() return 0 end, readonly }, -- always off now now
--# Created offset - 0x6030 Aircraft ground speed, double, in m/s.
[0x6030] = { fsuipc_types.float64, function() return ground_speed:read() end, readonly },
--# Custom offset - 0x66E0 CG Position Displacement from default in meters  (changed from previous 6700)
[0x66e0] = { fsuipc_types.float64, function() return cgz_ref_to_default:read() end, readonly },
--# Custom offset - 0x66E8 - stall warning Angle  of Attack - in Degrees
[0x66e8] = { fsuipc_types.float64, function() return acf_stall_warn_alpha:read() end, readonly },
--# Custom offset - 0x66F8 Engine 1 wash in m/s
[0x66f8] = { fsuipc_types.float64, function() return dvinc_0:read() end, readonly },
-- # Custom offset for trim value (to be removed!)
[0x66c4] = {fsuipc_types.sint16, function() return cl_trim end, function(value) cl_trim = value end},
-- # Custom offset for cl engage
[0x66c8] = {fsuipc_types.uint8, function() return 0 end, readonly},

--Stall warning (0=no, 1=stall)
[0x5300] = { fsuipc_types.uint8, function() return 0 end, readonly },

[0x036c] = { fsuipc_types.uint8, function() return stall_warning:read() end, readonly },
[0x0588] = { fsuipc_types.float64, function() return local_time_sec:read() end, readonly },
-- Aircraft on ground flag (0=airborne, 1=on ground). Not updated in Slew mode
--# Substitution offset - 0x0366 - On-ground indicator - hardcoded offset is not sensitive enough for vibration detection
[0x0366] = { fsuipc_types.uint16, function() return (gearF1:read() + gearF2:read() + gearF3:read() > 0) and 1 or 0 end, readonly },
--Engine 1 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
--# Substitute offset - to zero the fuel flow when engine 1 is not running
--# Does this by multiplying the fuel flow parameter by the engine running parameter (0 or 1)
[0x0918] = { fsuipc_types.float64, function() return eng1_running:read() * fuel_flow_1:read() * 7936.64 end, readonly },
-- Engine 2 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
[0x09b0] = { fsuipc_types.float64, function() return eng2_running:read() * fuel_flow_2:read() * 7936.64 end, readonly },
-- Engine 3 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
[0x0a48] = { fsuipc_types.float64, function() return eng3_running:read() * fuel_flow_3:read() * 7936.64 end, readonly },
-- Engine 4 Fuel Flow Pounds per Hour, as floating point double (FLOAT64)
[0x0ae0] = { fsuipc_types.float64, function() return eng4_running:read() * fuel_flow_4:read() * 7936.64 end, readonly },

-- Aileron trim value/control: �16383 to +16383
--# Custom offset - 0x0C02 Set Aileron Trim - data ref range +/1.0 scaled to +/- 16383
[0x0c02] = { fsuipc_types.sint16, function() return aileron_trim:read() * 16383 end, function(value)  aileron_trim:write(value / 16383 ) end },
-- Rudder trim value/control: �16383 to +16383
--# Custom offset - 0x0C04 Set Rudder Trim - data ref range +/1.0 scaled to +/- 16383
[0x0c04] = { fsuipc_types.sint16, function() return rudder_trim:read() * 16383 end, function(value)  rudder_trim:write(value / 16383 ) end },

--Engine 1 Jet N1 as 0 � 16384 (100%), or Prop RPM (derive RPM by multiplying this value by the RPM Scaler (see 08C8) and dividing by 65536). Note that Prop RPM is signed and negative for counter-rotating propellers.
[0x0898] = { fsuipc_types.uint16, function() return engine_rpm_0:read() *  16384 / 3000 end, readonly },
--Engine 2 Jet N1 as 0 � 16384 (100%), or Prop RPM (derive RPM by multiplying this value by the RPM Scaler (see 08C8) and dividing by 65536). Note that Prop RPM is signed and negative for counter-rotating propellers.
[0x0930] = { fsuipc_types.uint16, function() return engine_rpm_1:read() *  16384 / 3000 end, readonly },
--Engine 3 Jet N1 as 0 � 16384 (100%), or Prop RPM (derive RPM by multiplying this value by the RPM Scaler (see 08C8) and dividing by 65536). Note that Prop RPM is signed and negative for counter-rotating propellers.
[0x09c8] = { fsuipc_types.uint16, function() return engine_rpm_2:read() *  16384 / 3000 end, readonly },
--Engine 3 Jet N1 as 0 � 16384 (100%), or Prop RPM (derive RPM by multiplying this value by the RPM Scaler (see 08C8) and dividing by 65536). Note that Prop RPM is signed and negative for counter-rotating propellers.
[0x0a60] = { fsuipc_types.uint16, function() return engine_rpm_3:read() *  16384 / 3000 end, readonly },

--G Force: units unknown, but /624 seems to give quite sensible values. See also offset 1140
[0x11ba] = { fsuipc_types.sint16, function() return g_nrml:read() * 625 end, readonly },
-- Angle of Attack Indicator angle, with 360 degrees = 65536. The value 32767 is 180 degrees Angle of Attack. The angle is expressed in the usual FS 16-bit angle units (360 degrees = 65536), with 180 degrees pointing to the 0.0 position (right and down about 35 degrees in a Boeing type AofA indicator). Note that the indicator angle actually decreases as the wing AofA increases.
[0x11be] = { fsuipc_types.uint16, function()
   local aoa = aoa_degrees:read()
   if aoa < 0 then return 16384 end
   return (1 + aoa / acf_stall_warn_alpha:read()) / 2 * 65535 / 2 end, readonly },
--Fail mode, 0 ok, Hydraulics failure = 1
[0x0b62] = { fsuipc_types.uint8, function() return 0 end, readonly }, --  fixme: generalize
--Gear position (nose): 0=full up, 16383=full down
[0x0bec] = { fsuipc_types.uint32, function() return nose_gear_deploy_ratio:read() * 16383 end, readonly },
--Gear position (right): 0=full up, 16383=full down
[0x0bf0] = { fsuipc_types.uint32, function() return right_gear_deploy_ratio:read() * 16383 end, readonly },
--Gear position (left): 0=full up, 16383=full down
[0x0bf4] = { fsuipc_types.uint32, function() return left_gear_deploy_ratio:read() * 16383 end, readonly },
-- Incidence �alpha�, in radians, as a double (FLOAT64). This is the aircraft body angle of attack (AoA) not the wing AoA.
[0x2ed0] = { fsuipc_types.float64, function() return alpha:read() * 0.0174533 end, readonly },
-- Incidence �beta�, in radians, as a double (FLOAT64). This is the side slip angle.
[0x2ed8] = { fsuipc_types.float64, function() return beta:read() * 0.0174533 end, readonly },
-- TAS: True Air Speed, as knots * 128
[0x02b8] = { fsuipc_types.uint32, function() return true_airspeed:read() *  1.943844 * 128 end, readonly },
-- Vertical speed, signed, as 256 * metres/sec. For the more usual ft/min you need to apply the conversion *60*3.28084/256
[0x02c8] = { fsuipc_types.uint32, function() return vertical_speed:read() *  256 / 60 / 3.28084 end, readonly },
-- Minute of local time in FS (0�59)
[0x0239] = { fsuipc_types.uint8, function() return local_time_minutes:read() end, readonly },
-- Second of time in FS (0�59)
[0x023a] = { fsuipc_types.uint8, function() return local_time_seconds:read() end, readonly },
-- GS: Ground Speed, as 65536*metres/sec. Not updated in Slew mode!
[0x02b4] = { fsuipc_types.uint32, function() return ground_speed:read() * 65536 end, readonly },
-- Pause indicator (0=Not paused, 1=Paused)
[0x0264]={ fsuipc_types.uint16, function() return sim_paused:read() > 0 and 1 or 0 end, readonly},
-- Pitch
[0x0bb2]={ fsuipc_types.sint16, function() return yoke_pitch_ratio:read() * 16383 end, function(value) yoke_pitch_ratio:write(value / 16383 ) end },
-- Roll
-- Note: we READ roll to drive autopilot, but for REP arrow we cannot rely on yoke position, so read aileron positiion instead. We use LEFT aileron (FIXME)
[0x0bb6]={ fsuipc_types.sint16, function() return -lail1def:read() / ail1def_dn_max:read() * 16383 end, function(value)  yoke_roll_ratio:write(value / 16383 ) end },
-- Heading
[0x0bba] = { fsuipc_types.sint16, function() return yoke_heading_ratio:read() * 16383 end, function(value) yoke_heading_ratio:write(value / 16383) end },
-- Aileron deflection, in radians, as a double (FLOAT64). Right turn positive, left turn negative. (This is the average of left and right)
[0x2ea8] = { fsuipc_types.float64, function() return -lail1def:read() * 0.0174533 end, readonly },
-- Slew mode (indicator and control), 0=off, 1=on. (See 05DE also).
[0x05dc]={ fsuipc_types.uint16, function() return 0 end, function(value) end }, --fixme
-- Autopilot Master switch
[0x07bc]={ fsuipc_types.uint32, function() return ap_master:read() == 2 and 1 or 0 end, function(value) ap_master:write(value and 2 or 0) end },
-- Autopilot altitude lock
[0x07d0]={ fsuipc_types.uint32, function() return altitude_hold_status:read() ~= 0 and 1 or 0 end, readonly },
-- Crashed flag
[0x0840]={ fsuipc_types.sint16, function() return crashed:read() end,  readonly},
-- Elevator trim control input: �16383 to +16383
[0x0bc0]={ fsuipc_types.sint16,  function() return elevator_trim:read() * 16383 end, function(value)  elevator_trim:write(value / 16383 ) end },
-- Ready to Fly indicator. This is non-zero when FS is loading, or reloading a flight or aircraft or scenery, and
-- becomes zero when flight mode is enabled (even if the simulator is paused or in Slew mode).
[0x3364]={ fsuipc_types.uint8, function() return sim_paused:read() and 0 or 1 end, readonly},
-- In Menu or Dialog flag.
[0x3365]={ fsuipc_types.uint8, function() return sim_paused:read() end, readonly},
-- Propeller 1 thrust in pounds, as a double (FLOAT64). This is for props and turboprops.
--# FSUIPC offset - 0x2410 Engine 1 thrust in Pounds (comes in as Newtons)
[0x2410] = { fsuipc_types.float64, function() return point_thrust:read() * 0.2243 end, readonly },
-- Pitch velocity in rads/sec relative to the body axes in double floating point format.
[0x30a8] = { fsuipc_types.float64, function() return Qrad:read() end, readonly },
-- Roll acceleration in radians/sec/sec relative to the body in double floating point format.
[0x3080] = { fsuipc_types.float64, function() return P_dot:read() * 0.01745 end, readonly },
-- X (lateral, or left/right) acceleration in ft/sec/sec relative to the body axes in double floating point format.
[0x3060] = { fsuipc_types.float64, function() return g_side:read() * 3.28084 * 9.81 end, readonly },
-- Y (vertical, or up/down) acceleration in ft/sec/sec relative to the body axes in double floating point format.
[0x3068] = { fsuipc_types.float64, function() return g_nrml:read() * 3.28084 * 9.81 end, readonly },
--Z (longitudinal, or forward/backward) acceleration in ft/sec/sec
[0x3070] = { fsuipc_types.float64, function() return g_nrml:read() * 3.28084 * 9.81 end, readonly },
-- Pitch, *360/(65536*65536) for degrees. 0=level, �ve=pitch up, +ve=pitch down
[0x0578] = { fsuipc_types.sint32, function() return theta:read() * (65536 * 65536) / 360 end, readonly },
-- Ambient air density, in slugs per cubic foot, double floating point.
[0x28C0] = { fsuipc_types.float64, function() return rho:read() / 515.379 end, readonly },
-- Ambient air pressure, in lbs per square foot, double floating point
[0x28C8] = { fsuipc_types.float64, function() return barometer_current_inhg:read()  * 70.726 end, readonly },
-- Pressure Altitude (metres), double float.
-- FIXME!! : true altitude convert to pressure
[0x34B0] = { fsuipc_types.float64, function() return elevation:read() end, readonly },
-- Simulation rate *256 (i.e. 256=1x)
-- FIXME!! : always 1.0 for now
[0x0C1A] = { fsuipc_types.uint16, function() return 1 * 256 end, readonly },

-- Flaps position indicator (left). This gives the proportional amount, with 16383=full deflection.
-- fixme: now left is same as right
[0x0be0] = { fsuipc_types.uint32, function() return  flaps_deployment_ratio:read() * 16383 end, readonly },

-- Flaps position indicator (right). This gives the proportional amount, with 16383=full deflection.
-- fixme: now left is same as right
[0x0be4] = { fsuipc_types.uint32, function() return  flaps_deployment_ratio:read() * 16383 end, readonly },

-- Surface type as a 32-bit integer
-- x-plane
--surf_none        0,
--surf_water        1,
--surf_concrete        2,
--surf_asphalt   3,
--surf_grass        4,
--surf_dirt        5,
--surf_gravel        6,
--surf_lake        7,
--surf_snow        8,
--surf_shoulder        9,
--surf_blastpad        10,
--surf_grnd        11,
--surf_object        12
-- fixme: add surface handling

[0x31E8] = { fsuipc_types.uint32, function()

        local xplane2fsx_ground = { [4] = 1, [5] = 1, [6] = 1, [8] = 1,
                                    [11] = 2, [1] = 2, [7] = 2 }
        local surface =  surface_texture_type:read()
        local fsx_surface = xplane2fsx_ground[surface]
        if fsx_surface == nil then return 0 end
        return fsx_surface
    end, readonly },
[0x0E98] = { fsuipc_types.uint16, function() return turbulence_percent:read() * 255 end, readonly }

}
