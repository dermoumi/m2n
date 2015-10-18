local class = require 'nx.class'
local Nx = require 'nx'

local Timer = class 'nx.timer'

function Timer.static._fromCData(data)
    local ffi = require 'ffi'
    local timer = Timer:allocate()
    timer._cdata = ffi.cast('double*', data)
    return timer
end

function Timer:initialize()
    local ffi = require 'ffi'
    self._cdata = ffi.new('double[1]', {Nx.getSystemTime()})
end

function Timer:elapsedTime()
    return Nx.getSystemTime() - self._cdata[0]
end

function Timer:reset()
    self._cdata[0] = Nx.getSystemTime()
end

return Timer