/*//============================================================================
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
*///============================================================================
#include "../config.hpp"
#include "../audio/audio.hpp"

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSeek(uint32_t handle, double position)
{
    Audio::instance().seek(handle, position);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceStop(uint32_t handle)
{
    Audio::instance().stop(handle);
}

//----------------------------------------------------------
NX_EXPORT double nxAudioInstanceStreamTime(uint32_t handle)
{
    return Audio::instance().getStreamTime(handle);
}

//----------------------------------------------------------
NX_EXPORT bool nxAudioInstancePaused(uint32_t handle)
{
    return Audio::instance().getPause(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstanceVolume(uint32_t handle)
{
    return Audio::instance().getVolume(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstanceOverallVolume(uint32_t handle)
{
    return Audio::instance().getOverallVolume(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstancePan(uint32_t handle)
{
    return Audio::instance().getPan(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstanceSamplerate(uint32_t handle)
{
    return Audio::instance().getSamplerate(handle);
}

//----------------------------------------------------------
NX_EXPORT bool nxAudioInstanceProtected(uint32_t handle)
{
    return Audio::instance().getProtectVoice(handle);
}

//----------------------------------------------------------
NX_EXPORT bool nxAudioInstanceValid(uint32_t handle)
{
    return Audio::instance().isValidVoiceHandle(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstanceRelativePlaySpeed(uint32_t handle)
{
    return Audio::instance().getRelativePlaySpeed(handle);
}

//----------------------------------------------------------
NX_EXPORT bool nxAudioInstanceLooping(uint32_t handle)
{
    return Audio::instance().getLooping(handle);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetLooping(uint32_t handle, bool enabled)
{
    Audio::instance().setLooping(handle, enabled);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetInaudibleBehavior(uint32_t handle, bool tick, bool kill)
{
    Audio::instance().setInaudibleBehavior(handle, tick, kill);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetPaused(uint32_t handle, bool paused)
{
    Audio::instance().setPause(handle, paused);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetRelativePlaySpeed(uint32_t handle, float speed)
{
    Audio::instance().setRelativePlaySpeed(handle, speed);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetProtected(uint32_t handle, bool protect)
{
    Audio::instance().setProtectVoice(handle, protect);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetSamplerate(uint32_t handle, float samplerate)
{
    Audio::instance().setSamplerate(handle, samplerate);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetPan(uint32_t handle, float pan)
{
    Audio::instance().setPan(handle, pan);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetAbsolutePan(uint32_t handle, float l, float r, float lb, float rb,
    float c, float s)
{
    Audio::instance().setPanAbsolute(handle, l, r, lb, rb, c, s);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetVolume(uint32_t handle, float volume)
{
    Audio::instance().setVolume(handle, volume);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSetDelaySamples(uint32_t handle, uint32_t samples)
{
    Audio::instance().setDelaySamples(handle, samples);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceFadeVolume(uint32_t handle, float to, double t)
{
    Audio::instance().fadeVolume(handle, to, t);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceFadePan(uint32_t handle, float to, double t)
{
    Audio::instance().fadePan(handle, to, t);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceFadeRelativePlaySpeed(uint32_t handle, float to, double t)
{
    Audio::instance().fadeRelativePlaySpeed(handle, to, t);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSchedulePause(uint32_t handle, double t)
{
    Audio::instance().schedulePause(handle, t);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceScheduleStop(uint32_t handle, double t)
{
    Audio::instance().scheduleStop(handle, t);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioInstanceLoopCount(uint32_t handle)
{
    return Audio::instance().getLoopCount(handle);
}

//----------------------------------------------------------
NX_EXPORT float nxAudioInstanceInfo(uint32_t handle, uint32_t key)
{
    return Audio::instance().getInfo(handle, key);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourceParameters(uint32_t handle, float x, float y, float z,
    float velX, float velY, float velZ)
{
    Audio::instance().set3dSourceParameters(handle, x, y, z, velX, velY, velZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourcePosition(uint32_t handle, float x, float y, float z)
{
    Audio::instance().set3dSourcePosition(handle, x, y, z);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourceVelocity(uint32_t handle, float x, float y, float z)
{
    Audio::instance().set3dSourceVelocity(handle, x, y, z);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourceMinMaxDistance(uint32_t handle, float min, float max)
{
    Audio::instance().set3dSourceMinMaxDistance(handle, min, max);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourceAttenuation(uint32_t handle, uint32_t model,
    float rolloffFactor)
{
    Audio::instance().set3dSourceAttenuation(handle, model, rolloffFactor);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioInstanceSet3dSourceDopplerFactor(uint32_t handle, float factor)
{
    Audio::instance().set3dSourceDopplerFactor(handle, factor);
}
