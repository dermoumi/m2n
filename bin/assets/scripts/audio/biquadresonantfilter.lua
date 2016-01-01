--[[
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
--]]

local AudioFilter = require 'audio._filter'

local AudioBiquadResonantFilter = AudioFilter:subclass('audio.biquadresonantfilter')

local ffi = require 'ffi'
local C = ffi.C

local toFilterType = {
    [0] = 'none',
    [1] = 'lowpass',
    [2] = 'highpass',
    [3] = 'bandpass'
}

function AudioBiquadResonantFilter:initialize()
    local handle = C.nxAudioFilterBiquadResonantCreate()
    self._cdata = ffi.gc(handle, C.nxAudioFilterRelease)
end

function AudioBiquadResonantFilter:setParams(filterType, samplerate, frequency, resonance)
    if self._cdata ~= nil then
        C.nxAudioFilterBiquadResonantSetParams(
            self._cdata, toFilterType[filterType] or 0, samplerate, frequency, resonance
        )
    end
    
    return self
end

return AudioBiquadResonantFilter