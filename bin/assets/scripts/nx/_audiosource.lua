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
    typedef struct NxAudioSource NxAudioSource;

    void nxAudioSourceRelease(NxAudioSource*);
    int nxAudioPlay(NxAudioSource*, float, float, bool, uint32_t);
    int nxAudioPlayClocked(NxAudioSource*, double, float, float, uint32_t);
    int nxAudioPlay3d(NxAudioSource*, float, float, float, float, float, float, float, bool,
        uint32_t);
    int nxAudioPlay3dClocked(NxAudioSource*, double, float, float, float, float, float, float,
        float, uint32_t);
    void nxAudioSetVolume(NxAudioSource*, float);
    void nxAudioSetLooping(NxAudioSource*, bool);
    void nxAudioSetSingleInstance(NxAudioSource*, bool);
    void nxAudioSourceStop(NxAudioSource*);
    void nxAudioSetInaudibleBehavior(NxAudioSource*, bool, bool);
    void nxAudioSet3dMinMaxDistance(NxAudioSource*, float, float);
    void nxAudioSet3dAttenuation(NxAudioSource*, uint32_t, float);
    void nxAudioSet3dDopplerFactor(NxAudioSource*, float);
    void nxAudioSet3dListenerRelative(NxAudioSource*, bool);
    void nxAudioSet3dDistanceDelay(NxAudioSource*, bool);
]]

------------------------------------------------------------
local class = require 'nx.class'
local AudioSource = class 'nx._audiosource'

local AudioInstance = require 'nx._audioinstance'

------------------------------------------------------------
local toAttenuationModel = {
    [0] = 'none',
    [1] = 'inverse',
    [2] = 'linear',
    [3] = 'exponential'
}
AudioSource.static._toAttenuationModel = toAttenuationModel;

------------------------------------------------------------
function AudioSource:release()
    C.nxAudioSourceRelease(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function AudioSource:play(volume, pan, paused, bus)
    local handle = C.nxAudioPlay(self._cdata, volume or -1, pan or 0, not not paused, bus or 0)

    return AudioInstance:new(handle)
end

------------------------------------------------------------
function AudioSource:playClocked(interval, volume, pan, bus)
    local handle = C.nxAudioPlayClocked(
        self._cdata, interval, volume or -1, pan or 0, not not paused, bus or 0
    )

    return AudioInstance:new(handle)
end

------------------------------------------------------------
function AudioSource:play3d(x, y, z, velX, velY, velZ, volume, paused, bus)
    local handle = C.nxAudioPlay3d(
        self._cdata, x, y, z, velX or 1, velY or 1, velZ or 1, volume or -1, not not paused,
        bus or 0
    )

    return AudioInstance:new(handle)
end

------------------------------------------------------------
function AudioSource:play3dClocked(interval, x, y, z, velX, velY, velZ, volume, bus)
    local handle = C.nxAudioPlay3dClocked(
        self._cdata, interval, x, y, z, velX or 0, velY or 0, velZ or 0, volume or -1, bus or 0
    )

    return AudioInstance:new(handle)
end

------------------------------------------------------------
function AudioSource:setVolume(volume)
    C.nxAudioSetVolume(self._cdata, volume)
end

------------------------------------------------------------
function AudioSource:setLooping(looping)
    C.nxAudioSetLooping(self._cdata, looping)
end

------------------------------------------------------------
function AudioSource:setSingleInstance(singleInstance)
    C.nxAudioSetSingleInstance(self._cdata, singleInstance)
end

------------------------------------------------------------
function AudioSource:stop()
    C.nxAudioSourceStop(self._cdata)
end

------------------------------------------------------------
function AudioSource:setInaudibleBehavior(ticks, kill)
    C.nxAudioSetInaudibleBehavior(self._cdata, ticks, kill)
end

------------------------------------------------------------
function AudioSource:set3dMinMaxDistance(min, max)
    C.nxAudioSet3dMinMaxDistance(self._cdata, min, max)
end

------------------------------------------------------------
function AudioSource:set3dAttenuation(model, rolloffFactor)
    model = toAttenuationModel[model] or 0
    C.nxAudioSet3dAttenuation(self._cdata, model, rolloffFactor)
end

------------------------------------------------------------
function AudioSource:set3dDopplerFactor(factor)
    C.nxAudioSet3dDopplerFactor(self._cdata, factor)
end

------------------------------------------------------------
function AudioSource:set3dListenerRelative(enable)
    C.nxAudioSet3dListenerRelative(self._cdata, enable)
end

------------------------------------------------------------
function AudioSource:set3dDistanceDelay(enabled)
    C.nxAudioSet3dDistanceDelay(self._cdata, enabled)
end

------------------------------------------------------------
return AudioSource
