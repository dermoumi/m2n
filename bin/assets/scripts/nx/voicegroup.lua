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
    typedef struct {
        uint32_t handle;
    } NxVoiceGroup;

    NxVoiceGroup* nxAudioVoiceNewGroup();
    void nxAudioVoiceDestroyGroup(NxVoiceGroup*);
    void nxAudioVoiceAddToGroup(NxVoiceGroup*, uint32_t);
    bool nxAudioVoiceIsEmpty(NxVoiceGroup*);
]]

------------------------------------------------------------
local AudioVoice = require 'nx._audiovoice'
local VoiceGroup = AudioVoice:subclass('nx.voicegroup')

------------------------------------------------------------
function VoiceGroup.static._fromCData(data)
    local handle = VoiceGroup:allocate()
    handle._cdata = ffi.cast('NxVoiceGroup*', data)
    return handle
end

------------------------------------------------------------
function VoiceGroup:initialize()
    local handle = C.nxAudioVoiceNewGroup()
    self._cdata = ffi.gc(handle, C.nxAudioVoiceDestroyGroup)
end

------------------------------------------------------------
function VoiceGroup:_handle()
    return self._cdata.handle
end

------------------------------------------------------------
function VoiceGroup:isGroup()
    return true
end

------------------------------------------------------------
function VoiceGroup:add(voice)
    C.nxAudioVoiceAddToGroup(self._cdata, voice:_handle())
end

------------------------------------------------------------
function VoiceGroup:empty()
    return C.nxAudioVoiceIsEmpty(self._cdata)
end

------------------------------------------------------------
return VoiceGroup