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

#include <SDL2/SDL.h>

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxAudioFilter = SoLoud::Filter;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT bool nxAudioInit()
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return false;

    Audio::instance().init();
    return true;
}

//----------------------------------------------------------
NX_EXPORT void nxAudioRelease()
{
    Audio::instance().deinit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioStopAll()
{
    Audio::instance().stopAll();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioActiveVoiceCount()
{
    return Audio::instance().getActiveVoiceCount();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioVoiceCount()
{
    return Audio::instance().getVoiceCount();
}

//----------------------------------------------------------
NX_EXPORT float nxAudioPostClipScaler()
{
    return Audio::instance().getPostClipScaler();
}

//----------------------------------------------------------
NX_EXPORT float nxAudioGlobalVolume()
{
    return Audio::instance().getGlobalVolume();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioMaxActiveVoiceCount()
{
    return Audio::instance().getMaxActiveVoiceCount();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetMaxActiveVoiceCount(uint32_t voiceCount)
{
    Audio::instance().setMaxActiveVoiceCount(voiceCount);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetGlobalVolume(float volume)
{
    Audio::instance().setGlobalVolume(volume);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioPauseAll(bool pause)
{
    Audio::instance().setPauseAll(pause);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioFadeGlobalVolume(float to, double t)
{
    Audio::instance().fadeGlobalVolume(to, t);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioEnableVisualization(bool enabled)
{
    Audio::instance().setVisualizationEnable(enabled);
}

//----------------------------------------------------------
NX_EXPORT const float* nxAudioCalcFFTData()
{
    return Audio::instance().calcFFT();
}

//----------------------------------------------------------
NX_EXPORT const float* nxAudioCurrentWaveData()
{
    return Audio::instance().getWave();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioUpdate3dAudio()
{
    Audio::instance().update3dAudio();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dSoundSpeed(float speed)
{
    Audio::instance().set3dSoundSpeed(speed);
}

//----------------------------------------------------------
NX_EXPORT float nxAudio3dSoundSpeed()
{
    return Audio::instance().get3dSoundSpeed();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerParameters(float posX, float posY, float posZ, float atX,
    float atY, float atZ, float upX, float upY, float upZ, float velX, float velY, float velZ)
{
    Audio::instance().set3dListenerParameters(posX, posY, posZ, atX, atY, atZ, upX, upY, upZ,
        velX, velY, velZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerPosition(float posX, float posY, float posZ)
{
    Audio::instance().set3dListenerPosition(posX, posY, posZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerAt(float atX, float atY, float atZ)
{
    Audio::instance().set3dListenerAt(atX, atY, atZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerUp(float upX, float upY, float upZ)
{
    Audio::instance().set3dListenerUp(upX, upY, upZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerVelocity(float velX, float velY, float velZ)
{
    Audio::instance().set3dListenerVelocity(velX, velY, velZ);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetGlobalFilter(NxAudioFilter* filter, uint32_t id)
{
    Audio::instance().setGlobalFilter(id, filter);
}
