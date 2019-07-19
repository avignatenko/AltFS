local xplane = {}

xplane.types = {int=1, float=2, intarray=3, floatarray=4, string=5}

xplane.dataref = {}

local type2def = {[xplane.types.int] = 0, [xplane.types.float] = 0.0, [xplane.types.string]=""}

function xplane.dataref:new (_path, _type, _freq, _ops)
    o = {path=_path, type=_type, freq=_freq, ops=_ops}
    setmetatable(o, self)
    self.__index = self

	if o.ops == "r" or o.ops == "rw" then
	  xpl_dataref_subscribe(o.path, o.type, o.freq, function(value) o.value = value end)
	end

	o.value = type2def[_type]
    return o
end

function xplane.dataref:write(value)
    log(1, "write variable" .. self.path)
    xpl_dataref_write(self.path, self.type, value)
end

function xplane.dataref:read()
   log(1, "read variable" .. self.path)
   return self.value;
end

return xplane