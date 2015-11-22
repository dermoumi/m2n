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
    bool nxAudioInit();
    void nxAudioRelease();
    void nxAudioStopAll();
    uint32_t nxAudioActiveVoiceCount();
    uint32_t nxAudioVoiceCount();
    float nxAudioPostClipScaler();
    float nxAudioGlobalVolume();
    uint32_t nxAudioMaxActiveVoiceCount();
    void nxAudioSetMaxActiveVoiceCount(uint32_t);
    void nxAudioSetGlobalVolume(float);
    void nxAudioPauseAll(bool);
    void nxAudioFadeGlobalVolume(float, double);
    void nxAudioSetVisualizationEnabled(bool);
    const float* nxAudioCurrentWaveData();
    void nxAudioUpdate3dAudio();
    void nxAudioSet3dSoundSpeed(float);
    float nxAudio3dSoundSpeed();
    void nxAudioSet3dListenerParameters(float, float, float, float, float, float, float, float, float,
        float, float, float);
    void nxAudioSet3dListenerPosition(float, float, float);
    void nxAudioSet3dListenerAt(float, float, float);
    void nxAudioSet3dListenerUp(float, float, float);
    void nxAudioSet3dListenerVelocity(float, float, float);
]]

------------------------------------------------------------
local Audio = {}

------------------------------------------------------------
function Audio.init()
    return C.nxAudioInit()
end

------------------------------------------------------------
function Audio.release()
    C.nxAudioRelease()
end

------------------------------------------------------------
function Audio.stopAll()
    C.nxAudioStopAll()
end

------------------------------------------------------------
function Audio.pauseAll(pause)
    C.nxAudioPauseAll(pause)
end

------------------------------------------------------------
function Audio.setGlobalVolume(volume)
    C.nxAudioSetGlobalVolume(volume)
end

------------------------------------------------------------
function Audio.setMaxActiveVoiceCount(count)
    C.nxAudioSetMaxActiveVoiceCount(count)
end

------------------------------------------------------------
function Audio.setVisualizationEnabled(enabled)
    C.nxAudioSetVisualizationEnabled(enabled)
end

------------------------------------------------------------
function Audio.update3d()
    C.nxAudioUpdate3dAudio()
end

------------------------------------------------------------
function Audio.setSoundSpeed(speed)
    C.nxAudioSet3dSoundSpeed(speed)
end

------------------------------------------------------------
function Audio.setListenerParameters(
    posX, posY, posZ, atX, atY, atZ, upX, upY, upZ, velX, velY, velZ
)
    C.nxAudioSet3dListenerParameters(posX, posY, posZ, atX, atY, atZ, upX, upY, upZ, velX or 0,
        velY or 0, velZ or 0)
end

------------------------------------------------------------
function Audio.setListenerPosition(x, y, z)
    C.nxAudioSet3dListenerPosition(x, y, z)
end

------------------------------------------------------------
function Audio.setListenerAt(x, y, z)
    C.nxAudioSet3dListenerAt(x, y, z)
end

------------------------------------------------------------
function Audio.setListenerUp(x, y, z)
    C.nxAudioSet3dListenerUp(x, y, z)
end

------------------------------------------------------------
function Audio.setListenerVelocity(x, y, z)
    C.nxAudioSet3dListenerVelocity(x, y, z)
end

------------------------------------------------------------
function Audio.voiceCount(active)
    if active then
        return C.nxAudioActiveVoiceCount()
    else
        return C.nxAudioVoiceCount()
    end
end

------------------------------------------------------------
function Audio.postClipScaler()
    return C.nxAudioPostClipScaler()
end

------------------------------------------------------------
function Audio.globalVolume()
    return C.nxAudioGlobalVolume()
end

------------------------------------------------------------
function Audio.maxActiveVoiceCount()
    return C.nxAudioMaxActiveVoiceCount()
end

------------------------------------------------------------
function Audio.currentWaveData()
    return C.nxAudioCurrentWaveData()
end

------------------------------------------------------------
function Audio.soundSpeed()
    return C.nxAudio3dSoundSpeed()
end

------------------------------------------------------------
function Audio.fadeGlobalVolume(to, time)
    C.nxAudioFadeGlobalVolume(to, time)
end

------------------------------------------------------------
return Audio
