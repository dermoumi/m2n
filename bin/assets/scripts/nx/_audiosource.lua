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

local class      = require 'nx.class'
local AudioVoice = require 'nx._audiovoice'

local AudioSource = class 'nx._audiosource'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxAudioSource NxAudioSource;
    typedef struct NxAudioFilter NxAudioFilter;

    void nxAudioSourceRelease(NxAudioSource*);
    uint32_t nxAudioPlay(NxAudioSource*, float, float, bool);
    uint32_t nxAudioPlayClocked(NxAudioSource*, double, float, float);
    uint32_t nxAudioPlay3d(NxAudioSource*, float, float, float, float, float, float, float, bool);
    uint32_t nxAudioPlay3dClocked(NxAudioSource*, double, float, float, float, float, float, float,
        float);
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
    void nxAudioSourceSetFilter(NxAudioSource*, NxAudioFilter*, uint32_t);

    NxAudioSource* nxAudioBusCreate();
    uint32_t nxAudioSourcePlayThrough(NxAudioSource*, NxAudioSource*, float, float, bool);
    uint32_t nxAudioSourcePlayClockedThrough(NxAudioSource*, NxAudioSource*, double, float, float);
    uint32_t nxAudioSourcePlay3dThrough(NxAudioSource*, NxAudioSource*, float, float, float, float,
        float, float, float, bool);
    uint32_t nxAudioSourcePlay3dClockedThrough(NxAudioSource*, NxAudioSource*, double, float, float,
        float, float, float, float, float);
    void nxAudioBusSetChannels(NxAudioSource*, uint32_t);
    void nxAudioBusEnableVisualization(NxAudioSource*, bool);
    const float* nxAudioBusCalcFFT(NxAudioSource*);
    const float* nxAudioBusCurrentWaveData(NxAudioSource*);
]]

------------------------------------------------------------
local toAttenuationModel = {
    none        = 0,
    inverse     = 1,
    linear      = 2,
    exponential = 3
}
AudioSource.static._toAttenuationModel = toAttenuationModel;

------------------------------------------------------------
function AudioSource:release()
    if self._cdata == nil then return end
    C.nxAudioSourceRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

------------------------------------------------------------
function AudioSource:play(volume, pan, paused)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioPlay(self._cdata, volume or -1, pan or 0, not not paused)

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:playClocked(interval, volume, pan)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioPlayClocked(
        self._cdata, interval, volume or -1, pan or 0, not not paused
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:play3d(x, y, z, velX, velY, velZ, volume, paused)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioPlay3d(
        self._cdata, x, y, z, velX or 1, velY or 1, velZ or 1, volume or -1, not not paused
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:play3dClocked(interval, x, y, z, velX, velY, velZ, volume)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioPlay3dClocked(
        self._cdata, interval, x, y, z, velX or 0, velY or 0, velZ or 0, volume or -1
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:playThrough(bus, volume, pan, paused)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioSourcePlayThrough(
        self._cdata, bus._cdata, volume or -1, pan or 0, not not paused
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:playClockedThrough(bus, interval, volume, pan)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioSourcePlayClockedThrough(
        self._cdata, bus._cdata, interval, volume or -1, pan or 0
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:play3dThrough(bus, x, y, z, velX, velY, velZ, volume, paused)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioSourcePlay3dThrough(
        self._cdata, bus._cdata, x, y, z, velX or 0, velY or 0, velZ or 0, volume or -1,
        not not paused
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:play3dClockedThrough(bus, interval, x, y, z, velX, velY, velZ, volume)
    if self._cdata == nil then return AudioVoice:new(0) end
    local handle = C.nxAudioSourcePlay3dClockedThrough(
        self._cdata, bus._cdata, interval, x, y, z, velX or 0, velY or 0, velZ or 0, volume or -1
    )

    return AudioVoice:new(handle)
end

------------------------------------------------------------
function AudioSource:setVolume(volume)
    if self._cdata ~= nil then
        C.nxAudioSetVolume(self._cdata, volume)
    end

    return self
end

------------------------------------------------------------
function AudioSource:setLooping(looping)
    if self._cdata ~= nil then
        C.nxAudioSetLooping(self._cdata, looping)
    end

    return self
end

------------------------------------------------------------
function AudioSource:setSingleInstance(singleInstance)
    if self._cdata ~= nil then
        C.nxAudioSetSingleInstance(self._cdata, singleInstance)
    end

    return self
end

------------------------------------------------------------
function AudioSource:stop()
    if self._cdata ~= nil then
        C.nxAudioSourceStop(self._cdata)
    end

    return self
end

------------------------------------------------------------
function AudioSource:setInaudibleBehavior(ticks, kill)
    if self._cdata ~= nil then
        C.nxAudioSetInaudibleBehavior(self._cdata, ticks, kill)
    end

    return self
end

------------------------------------------------------------
function AudioSource:set3dMinMaxDistance(min, max)
    if self._cdata ~= nil then
        C.nxAudioSet3dMinMaxDistance(self._cdata, min, max)
    end

    return self
end

------------------------------------------------------------
function AudioSource:set3dAttenuation(model, rolloffFactor)
    if self._cdata ~= nil then
        C.nxAudioSet3dAttenuation(self._cdata, toAttenuationModel[model] or 0, rolloffFactor)
    end

    return self
end

------------------------------------------------------------
function AudioSource:set3dDopplerFactor(factor)
    if self._cdata ~= nil then
        C.nxAudioSet3dDopplerFactor(self._cdata, factor)
    end

    return self
end

------------------------------------------------------------
function AudioSource:set3dListenerRelative(enable)
    if self._cdata ~= nil then
        C.nxAudioSet3dListenerRelative(self._cdata, enable)
    end

    return self
end

------------------------------------------------------------
function AudioSource:set3dDistanceDelay(enabled)
    if self._cdata ~= nil then
        C.nxAudioSet3dDistanceDelay(self._cdata, enabled)
    end

    return self
end

------------------------------------------------------------
function AudioSource:setFilter(filter, id)
    if self._cdata ~= nil then
        if filter then filter = filter._cdata end
        C.nxAudioSourceSetFilter(self._cdata, filter, id or 0)
    end

    return self
end

------------------------------------------------------------
return AudioSource