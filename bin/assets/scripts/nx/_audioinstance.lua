--[[----------------------------------------------------------------------------
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
--]]----------------------------------------------------------------------------

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    void nxAudioInstanceSeek(uint32_t, double);
    void nxAudioInstanceStop(uint32_t);
    double nxAudioInstanceStreamTime(uint32_t);
    bool nxAudioInstancePaused(uint32_t);
    float nxAudioInstanceVolume(uint32_t);
    float nxAudioInstanceOverallVolume(uint32_t);
    float nxAudioInstancePan(uint32_t);
    float nxAudioInstanceSamplerate(uint32_t);
    bool nxAudioInstanceProtected(uint32_t);
    bool nxAudioInstanceValid(uint32_t);
    float nxAudioInstanceRelativePlaySpeed(uint32_t);
    bool nxAudioInstanceLooping(uint32_t);
    void nxAudioInstanceSetLooping(uint32_t, bool);
    void nxAudioInstanceSetInaudibleBehavior(uint32_t, bool, bool);
    void nxAudioInstanceSetPaused(uint32_t, bool);
    void nxAudioInstanceSetRelativePlaySpeed(uint32_t, float);
    void nxAudioInstanceSetProtected(uint32_t, bool);
    void nxAudioInstanceSetSamplerate(uint32_t, float);
    void nxAudioInstanceSetPan(uint32_t, float);
    void nxAudioInstanceSetAbsolutePan(uint32_t, float, float, float, float, float, float);
    void nxAudioInstanceSetVolume(uint32_t, float);
    void nxAudioInstanceSetDelaySamples(uint32_t, uint32_t);
    void nxAudioInstanceFadeVolume(uint32_t, float, double);
    void nxAudioInstanceFadePan(uint32_t, float, double);
    void nxAudioInstanceFadeRelativePlaySpeed(uint32_t, float, double);
    void nxAudioInstanceSchedulePause(uint32_t, double);
    void nxAudioInstanceScheduleStop(uint32_t, double);
    uint32_t nxAudioInstanceLoopCount(uint32_t);
    float nxAudioInstanceInfo(uint32_t, uint32_t);
    void nxAudioInstanceSet3dSourceParameters(uint32_t, float, float, float, float, float, float);
    void nxAudioInstanceSet3dSourcePosition(uint32_t, float, float, float);
    void nxAudioInstanceSet3dSourceVelocity(uint32_t, float, float, float);
    void nxAudioInstanceSet3dSourceMinMaxDistance(uint32_t, float, float);
    void nxAudioInstanceSet3dSourceAttenuation(uint32_t, uint32_t, float);
    void nxAudioInstanceSet3dSourceDopplerFactor(uint32_t, float);
]]

------------------------------------------------------------
local class = require 'nx.class'
local AudioInstance = class 'nx._audioinstance'

------------------------------------------------------------
function AudioInstance.static._fromCData(data)
    return AudioInstance:new(data)
end

------------------------------------------------------------
function AudioInstance:initialize(handle)
    self._cdata = handle
end

------------------------------------------------------------
function AudioInstance:seek(pos)
    C.nxAudioInstanceSeek(self._cdata, pos)
end

------------------------------------------------------------
function AudioInstance:pause(paused)
    C.nxAudioInstanceSetPaused(self._cdata, paused)
end

------------------------------------------------------------
function AudioInstance:stop()
    C.nxAudioInstanceStop(self._cdata)
end

------------------------------------------------------------
function AudioInstance:protect(protected)
    C.nxAudioInstanceSetProtected(self._cdata, protected)
end

------------------------------------------------------------
function AudioInstance:setLooping(enabled)
    C.nxAudioInstanceSetLooping(self._cdata, enabled)
end

------------------------------------------------------------
function AudioInstance:setInaudibleBehavior(tick, kill)
    C.nxAudioInstanceSetInaudibleBehavior(self._cdata, tick, kill)
end

------------------------------------------------------------
function AudioInstance:setPlaySpeed(factor)
    C.nxAudioInstanceSetRelativePlaySpeed(self._cdata, factor)
end

------------------------------------------------------------
function AudioInstance:setSamplerate(samplerate)
    C.nxAudioInstanceSetSamplerate(self._cdata, samplerate)
end

------------------------------------------------------------
function AudioInstance:setPan(pan)
    C.nxAudioInstanceSetPan(self._cdata, pan)
end

------------------------------------------------------------
function AudioInstance:setAbsolutePan(l, r, lb, rb, c, s)
    C.nxAudioInstanceSetAbsolutePan(self._cdata, l, r, lb or 0, rb or 0, c or 0, s or 0)
end

------------------------------------------------------------
function AudioInstance:setVolume(vol)
    C.nxAudioInstanceSetVolume(self._cdata, vol)
end

------------------------------------------------------------
function AudioInstance:setDelaySamples(samples)
    C.nxAudioInstanceSetDelaySamples(self._cdata, samples)
end

------------------------------------------------------------
function AudioInstance:set3dParameters(x, y, z, velX, velY, velZ)
    C.nxAudioInstanceSet3dSourceParameters(self._cdata, x, y, z, velX or 0, velY or 0, velZ or 0)
end

------------------------------------------------------------
function AudioInstance:set3dPosition(x, y, z)
    C.nxAudioInstanceSet3dSourcePosition(self._cdata, x, y, z)
end

------------------------------------------------------------
function AudioInstance:set3dVelocity(x, y, z)
    C.nxAudioInstanceSet3dSourceVelocity(self._cdata, x, y, z)
end

------------------------------------------------------------
function AudioInstance:set3dMinMaxDistance(min, max)
    C.nxAudioInstanceSet3dSourceMinMaxDistance(self._cdata, min, max)
end

------------------------------------------------------------
function AudioInstance:set3dAttenuation(model, rolloffFactor)
    model = require('nx._audiosource')._toAttenuationModel[model] or 0
    C.nxAudioInstanceSet3dSourceAttenuation(self._cdata, model, rolloffFactor)
end

------------------------------------------------------------
function AudioInstance:set3dDopplerFactor(factor)
    C.nxAudioInstanceSet3dSourceDopplerFactor(self._cdata, factor)
end

------------------------------------------------------------
function AudioInstance:time()
    return C.nxAudioInstanceStreamTime(self._cdata)
end

------------------------------------------------------------
function AudioInstance:paused()
    return C.nxAudioInstancePaused(self._cdata)
end

------------------------------------------------------------
function AudioInstance:volume()
    return C.nxAudioInstanceVolume(self._cdata)
end

------------------------------------------------------------
function AudioInstance:overallVolume()
    return C.nxAudioInstanceOverallVolume(self._cdata);
end

------------------------------------------------------------
function AudioInstance:pan()
    return C.nxAudioInstancePan(self._cdata)
end

------------------------------------------------------------
function AudioInstance:samplerate()
    return C.nxAudioInstanceSamplerate(self._cdata)
end

------------------------------------------------------------
function AudioInstance:protected()
    return C.nxAudioInstanceProtected(self._cdata)
end

------------------------------------------------------------
function AudioInstance:valid()
    return C.nxAudioInstanceValid(self._cdata)
end

------------------------------------------------------------
function AudioInstance:playSpeed()
    return C.nxAudioInstanceRelativePlaySpeed(self._cdata)
end

------------------------------------------------------------
function AudioInstance:looping()
    return C.nxAudioInstanceLooping(self._cdata)
end

------------------------------------------------------------
function AudioInstance:loopCount()
    return C.nxAudioInstanceLoopCount(self._cdata)
end

------------------------------------------------------------
function AudioInstance:info(key)
    return C.nxAudioInstanceInfo(self._cdata, key)
end

------------------------------------------------------------
function AudioInstance:fadeVolume(to, time)
    C.nxAudioInstanceFadeVolume(self._cdata, to, time)
end

------------------------------------------------------------
function AudioInstance:fadePan(to, time)
    C.nxAudioInstanceFadePan(self._cdata, to, time)
end

------------------------------------------------------------
function AudioInstance:fadePlaySpeed(to, time)
    C.nxAudioInstanceFadeRelativePlaySpeed(self._cdata, to, time)
end

------------------------------------------------------------
function AudioInstance:schedulePause(time)
    C.nxAudioInstanceSchedulePause(self._cdata, time)
end

------------------------------------------------------------
function AudioInstance:scheduleStop(time)
    C.nxAudioInstanceScheduleStop(self._cdata, stop)
end

------------------------------------------------------------
return AudioInstance