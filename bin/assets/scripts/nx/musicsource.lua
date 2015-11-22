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
-- Represents an streamable sound file
------------------------------------------------------------
local AudioSource = require 'nx._audiosource'
local MusicSource = AudioSource:subclass('nx.musicsource')

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    NxAudioSource* nxAudioMusicCreate();
    void nxAudioMusicOpenFile(NxAudioSource*, const char*);
    void nxAudioMusicOpenMemory(NxAudioSource*, uint8_t* buffer, size_t size);
    double nxAudioMusicLength(NxAudioSource*);
]]

------------------------------------------------------------
function MusicSource.static._fromCData(cdata)
    local musicSource = MusicSource:allocate()
    musicSource._cdata = ffi.cast('NxAudioSource*', cdata)
    return musicSource
end

------------------------------------------------------------
function MusicSource:initialize()
    local handle = C.nxAudioMusicCreate()
    self._cdata = ffi.gc(handle, C.nxAudioSourceRelease)
end

------------------------------------------------------------
function MusicSource:open(a, b)
    if b then
        C.nxAudioMusicOpenMemory(self._cdata, a, b)
    else
        C.nxAudioMusicOpenFile(self._cdata, a)
    end
end

------------------------------------------------------------
function MusicSource:length()
    return C.nxAudioMusicLength(self._cdata)
end

------------------------------------------------------------
return MusicSource
