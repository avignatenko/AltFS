types = {uint8=1, uint16=2, uint32=3, uint64=4, sint8=5, sint16=6, sint32=7, sint64=8, float32=9, float64=10}

offsets=
{
[0x225]={
  type = types.uint8, 
  read = function() end,  
  write = function(value) end
  },
  [0x226]={
  type = types.uint16, 
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