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
-- Represents an in-memory sound file
------------------------------------------------------------
local AudioSource = require 'nx._audiosource'
local SoundSource = AudioSource:subclass('nx.soundsource')

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    NxAudioSource* nxAudioSoundCreate();
    void nxAudioSoundOpenFile(NxAudioSource*, const char*);
    void nxAudioSoundOpenMemory(NxAudioSource*, uint8_t*, size_t);
]]

------------------------------------------------------------
function SoundSource.static._fromCData(cdata)
    local soundSource = SoundSource:allocate()
    soundSource._cdata = ffi.cast('NxAudioSource*', cdata)
    return soundSource
end

------------------------------------------------------------
function SoundSource:initialize()
    local handle = C.nxAudioSoundCreate()
    self._cdata = ffi.gc(handle, C.nxAudioRelease)
end

------------------------------------------------------------
function SoundSource:open(a, b)
    if b then
        C.nxAudioSoundOpenMemory(self._cdata, a, b)
    else
        C.nxAudioSoundOpenFile(self._cdata, a)
    end
end

------------------------------------------------------------
return SoundSource