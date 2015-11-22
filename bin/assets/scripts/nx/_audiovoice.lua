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
    void nxAudioVoiceSeek(uint32_t, double);
    void nxAudioVoiceStop(uint32_t);
    double nxAudioVoiceStreamTime(uint32_t);
    bool nxAudioVoicePaused(uint32_t);
    float nxAudioVoiceVolume(uint32_t);
    float nxAudioVoiceOverallVolume(uint32_t);
    float nxAudioVoicePan(uint32_t);
    float nxAudioVoiceSamplerate(uint32_t);
    bool nxAudioVoiceProtected(uint32_t);
    bool nxAudioVoiceValid(uint32_t);
    float nxAudioVoiceRelativePlaySpeed(uint32_t);
    bool nxAudioVoiceLooping(uint32_t);
    void nxAudioVoiceSetLooping(uint32_t, bool);
    void nxAudioVoiceSetInaudibleBehavior(uint32_t, bool, bool);
    void nxAudioVoiceSetPaused(uint32_t, bool);
    void nxAudioVoiceSetRelativePlaySpeed(uint32_t, float);
    void nxAudioVoiceSetProtected(uint32_t, bool);
    void nxAudioVoiceSetSamplerate(uint32_t, float);
    void nxAudioVoiceSetPan(uint32_t, float);
    void nxAudioVoiceSetAbsolutePan(uint32_t, float, float, float, float, float, float);
    void nxAudioVoiceSetVolume(uint32_t, float);
    void nxAudioVoiceSetDelaySamples(uint32_t, uint32_t);
    void nxAudioVoiceFadeVolume(uint32_t, float, double);
    void nxAudioVoiceFadePan(uint32_t, float, double);
    void nxAudioVoiceFadeRelativePlaySpeed(uint32_t, float, double);
    void nxAudioVoiceSchedulePause(uint32_t, double);
    void nxAudioVoiceScheduleStop(uint32_t, double);
    uint32_t nxAudioVoiceLoopCount(uint32_t);
    float nxAudioVoiceInfo(uint32_t, uint32_t);
    void nxAudioVoiceSet3dSourceParameters(uint32_t, float, float, float, float, float, float);
    void nxAudioVoiceSet3dSourcePosition(uint32_t, float, float, float);
    void nxAudioVoiceSet3dSourceVelocity(uint32_t, float, float, float);
    void nxAudioVoiceSet3dSourceMinMaxDistance(uint32_t, float, float);
    void nxAudioVoiceSet3dSourceAttenuation(uint32_t, uint32_t, float);
    void nxAudioVoiceSet3dSourceDopplerFactor(uint32_t, float);
    void nxAudioVoiceSetFilterParameter(uint32_t, uint32_t, uint32_t, float);
    float nxAudioVoiceFilterParamter(uint32_t, uint32_t, uint32_t);
    void nxAudioVoiceFadeFilterParameter(uint32_t, uint32_t, uint32_t, float, float, double);
]]

------------------------------------------------------------
local class = require 'nx.class'
local AudioVoice = class 'nx._audiovoice'

------------------------------------------------------------
function AudioVoice.static._fromCData(data)
    return AudioVoice:new(data)
end

------------------------------------------------------------
function AudioVoice:initialize(handle)
    self._cdata = handle
end

------------------------------------------------------------
function AudioVoice:_handle()
    return self._cdata
end

------------------------------------------------------------
function AudioVoice:isGroup()
    return false
end

------------------------------------------------------------
function AudioVoice:seek(pos)
    C.nxAudioVoiceSeek(self:_handle(), pos)
end

------------------------------------------------------------
function AudioVoice:pause(paused)
    C.nxAudioVoiceSetPaused(self:_handle(), paused)
end

------------------------------------------------------------
function AudioVoice:stop()
    C.nxAudioVoiceStop(self:_handle())
end

------------------------------------------------------------
function AudioVoice:protect(protected)
    C.nxAudioVoiceSetProtected(self:_handle(), protected)
end

------------------------------------------------------------
function AudioVoice:setLooping(enabled)
    C.nxAudioVoiceSetLooping(self:_handle(), enabled)
end

------------------------------------------------------------
function AudioVoice:setInaudibleBehavior(tick, kill)
    C.nxAudioVoiceSetInaudibleBehavior(self:_handle(), tick, kill)
end

------------------------------------------------------------
function AudioVoice:setPlaySpeed(factor)
    C.nxAudioVoiceSetRelativePlaySpeed(self:_handle(), factor)
end

------------------------------------------------------------
function AudioVoice:setSamplerate(samplerate)
    C.nxAudioVoiceSetSamplerate(self:_handle(), samplerate)
end

------------------------------------------------------------
function AudioVoice:setPan(pan)
    C.nxAudioVoiceSetPan(self:_handle(), pan)
end

------------------------------------------------------------
function AudioVoice:setAbsolutePan(l, r, lb, rb, c, s)
    C.nxAudioVoiceSetAbsolutePan(self:_handle(), l, r, lb or 0, rb or 0, c or 0, s or 0)
end

------------------------------------------------------------
function AudioVoice:setVolume(vol)
    C.nxAudioVoiceSetVolume(self:_handle(), vol)
end

------------------------------------------------------------
function AudioVoice:setDelaySamples(samples)
    C.nxAudioVoiceSetDelaySamples(self:_handle(), samples)
end

------------------------------------------------------------
function AudioVoice:set3dParameters(x, y, z, velX, velY, velZ)
    C.nxAudioVoiceSet3dSourceParameters(self:_handle(), x, y, z, velX or 0, velY or 0, velZ or 0)
end

------------------------------------------------------------
function AudioVoice:set3dPosition(x, y, z)
    C.nxAudioVoiceSet3dSourcePosition(self:_handle(), x, y, z)
end

------------------------------------------------------------
function AudioVoice:set3dVelocity(x, y, z)
    C.nxAudioVoiceSet3dSourceVelocity(self:_handle(), x, y, z)
end

------------------------------------------------------------
function AudioVoice:set3dMinMaxDistance(min, max)
    C.nxAudioVoiceSet3dSourceMinMaxDistance(self:_handle(), min, max)
end

------------------------------------------------------------
function AudioVoice:set3dAttenuation(model, rolloffFactor)
    model = require('nx._audiosource')._toAttenuationModel[model] or 0
    C.nxAudioVoiceSet3dSourceAttenuation(self:_handle(), model, rolloffFactor)
end

------------------------------------------------------------
function AudioVoice:set3dDopplerFactor(factor)
    C.nxAudioVoiceSet3dSourceDopplerFactor(self:_handle(), factor)
end

------------------------------------------------------------
function AudioVoice:setFilterParameter(id, attribute, value)
    C.nxAudioVoiceSetFilterParameter(self:_handle(), id, attribute, value)
end

------------------------------------------------------------
function AudioVoice:time()
    return C.nxAudioVoiceStreamTime(self:_handle())
end

------------------------------------------------------------
function AudioVoice:paused()
    return C.nxAudioVoicePaused(self:_handle())
end

------------------------------------------------------------
function AudioVoice:volume()
    return C.nxAudioVoiceVolume(self:_handle())
end

------------------------------------------------------------
function AudioVoice:overallVolume()
    return C.nxAudioVoiceOverallVolume(self:_handle());
end

------------------------------------------------------------
function AudioVoice:pan()
    return C.nxAudioVoicePan(self:_handle())
end

------------------------------------------------------------
function AudioVoice:samplerate()
    return C.nxAudioVoiceSamplerate(self:_handle())
end

------------------------------------------------------------
function AudioVoice:protected()
    return C.nxAudioVoiceProtected(self:_handle())
end

------------------------------------------------------------
function AudioVoice:valid()
    return C.nxAudioVoiceValid(self:_handle())
end

------------------------------------------------------------
function AudioVoice:playSpeed()
    return C.nxAudioVoiceRelativePlaySpeed(self:_handle())
end

------------------------------------------------------------
function AudioVoice:looping()
    return C.nxAudioVoiceLooping(self:_handle())
end

------------------------------------------------------------
function AudioVoice:loopCount()
    return C.nxAudioVoiceLoopCount(self:_handle())
end

------------------------------------------------------------
function AudioVoice:info(key)
    return C.nxAudioVoiceInfo(self:_handle(), key)
end

------------------------------------------------------------
function AudioVoice:filterParameter(id, attribute)
    return C.nxAudioVoiceFilterParamter(self:_handle(), id, attribute)
end

------------------------------------------------------------
function AudioVoice:fadeVolume(to, time)
    C.nxAudioVoiceFadeVolume(self:_handle(), to, time)
end

------------------------------------------------------------
function AudioVoice:fadePan(to, time)
    C.nxAudioVoiceFadePan(self:_handle(), to, time)
end

------------------------------------------------------------
function AudioVoice:fadePlaySpeed(to, time)
    C.nxAudioVoiceFadeRelativePlaySpeed(self:_handle(), to, time)
end

------------------------------------------------------------
function AudioVoice:fadeFilterParameter(id, attribute, from, to, time)
    C.nxAudioVoiceFadeFilterParameter(self:_handle(), id, attribute, from, to, time)
end

------------------------------------------------------------
function AudioVoice:schedulePause(time)
    C.nxAudioVoiceSchedulePause(self:_handle(), time)
end

------------------------------------------------------------
function AudioVoice:scheduleStop(time)
    C.nxAudioVoiceScheduleStop(self:_handle(), stop)
end

------------------------------------------------------------
return AudioVoice