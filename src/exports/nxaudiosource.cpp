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

#include <soloud/soloud_wav.h>
#include <soloud/soloud_wavstream.h>

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxAudioFilter = SoLoud::Filter;
struct NxAudioSource
{
    SoLoud::AudioSource* handle {nullptr};
    Audio::File* file {nullptr}; // Used if the audio source needs an active file
};

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxAudioSource* nxAudioSourceCreate()
{
    return new NxAudioSource();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceRelease(NxAudioSource* source)
{
    delete source->handle;
    delete source->file;
    delete source;
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceLoadFile(NxAudioSource* source, const char* filename)
{
    delete source->file;
    source->file = nullptr;

    Audio::File file;
    file.open(filename);

    delete static_cast<SoLoud::Wav*>(source->handle);
    source->handle = new SoLoud::Wav();
    static_cast<SoLoud::Wav*>(source->handle)->loadFile(&file);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceLoadMemory(NxAudioSource* source, uint8_t* buffer, size_t size)
{
    delete source->file;
    source->file = nullptr;

    delete static_cast<SoLoud::Wav*>(source->handle);
    source->handle = new SoLoud::Wav();
    static_cast<SoLoud::Wav*>(source->handle)->loadMem(buffer, size, true, false);
}

//----------------------------------------------------------
NX_EXPORT double nxAudioSourceStaticLength(NxAudioSource* source)
{
    return static_cast<SoLoud::Wav*>(source->handle)->getLength();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceOpenFile(NxAudioSource* source, const char* filename)
{
    delete source->file;
    source->file = new Audio::File();
    source->file->open(filename);

    delete static_cast<SoLoud::WavStream*>(source->handle);
    source->handle = new SoLoud::WavStream();
    static_cast<SoLoud::WavStream*>(source->handle)->loadFile(source->file);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceOpenMemory(NxAudioSource* source, uint8_t* buffer, size_t size)
{
    delete source->file;
    source->file = nullptr;

    delete static_cast<SoLoud::WavStream*>(source->handle);
    source->handle = new SoLoud::WavStream();
    static_cast<SoLoud::WavStream*>(source->handle)->loadMem(buffer, size, true, false);
}

//----------------------------------------------------------
NX_EXPORT double nxAudioSourceStreamLength(NxAudioSource* source)
{
    return static_cast<SoLoud::WavStream*>(source->handle)->getLength();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioPlay(NxAudioSource* source, float volume, float pan, bool paused)
{
    return Audio::instance().play(*source->handle, volume, pan, paused);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioPlayClocked(NxAudioSource* source, double interval, float volume,
    float pan)
{
    return Audio::instance().playClocked(interval, *source->handle, volume, pan);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioPlay3d(NxAudioSource* source, float x, float y, float z, float velX,
    float velY, float velZ, float volume, bool paused)
{
    return Audio::instance().play3d(
        *source->handle, x, y, z, velX, velY, velZ, volume, paused
    );
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioPlay3dClocked(NxAudioSource* source, double interval, float x, float y,
    float z, float velX, float velY, float velZ, float volume)
{
    return Audio::instance().play3dClocked(
        interval, *source->handle, x, y, z, velX, velY, velZ, volume
    );
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetVolume(NxAudioSource* source, float volume)
{
    source->handle->setVolume(volume);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetLooping(NxAudioSource* source, bool looping)
{
    source->handle->setLooping(looping);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetSingleInstance(NxAudioSource* source, bool singleInstance)
{
    source->handle->setSingleInstance(singleInstance);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceStop(NxAudioSource* source)
{
    source->handle->stop();
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSetInaudibleBehavior(NxAudioSource* source, bool ticks, bool kill)
{
    source->handle->setInaudibleBehavior(ticks, kill);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dMinMaxDistance(NxAudioSource* source, float min, float max)
{
    source->handle->set3dMinMaxDistance(min, max);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dAttenuation(NxAudioSource* source, uint32_t model, float rolloffFactor)
{
    source->handle->set3dAttenuation(model, rolloffFactor);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dDopplerFactor(NxAudioSource* source, float dopplerFactor)
{
    source->handle->set3dDopplerFactor(dopplerFactor);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dListenerRelative(NxAudioSource* source, bool relative)
{
    source->handle->set3dListenerRelative(relative);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSet3dDistanceDelay(NxAudioSource* source, bool delay)
{
    source->handle->set3dDistanceDelay(delay);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSourceSetFilter(NxAudioSource* source, NxAudioFilter* filter, uint32_t id)
{
    source->handle->setFilter(id, filter);
}

//----------------------------------------------------------
NX_EXPORT NxAudioSource* nxAudioBusCreate()
{
    auto source = new NxAudioSource();
    source->handle = new SoLoud::Bus();
    return source;
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioSourcePlayThrough(NxAudioSource* source, NxAudioSource* bus, float volume,
    float pan, bool paused)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->play(*source->handle, volume, pan, paused);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioSourcePlayClockedThrough(NxAudioSource* source, NxAudioSource* bus,
    double interval, float volume, float pan)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->playClocked(
        interval, *source->handle, volume, pan
    );
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioSourcePlay3dThrough(NxAudioSource* source, NxAudioSource* bus,
    float posX, float posY, float posZ, float velX, float velY, float velZ, float volume,
    bool paused)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->play3d(
        *source->handle, posX, posY, posZ, velX, velY, velZ, volume, paused
    );
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxAudioSourcePlay3dClockedThrough(NxAudioSource* source, NxAudioSource* bus,
    double interval, float posX, float posY, float posZ, float velX, float velY, float velZ,
    float volume)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->play3dClocked(
        interval, *source->handle, posX, posY, posZ, velX, velY, velZ, volume
    );
}

//----------------------------------------------------------
NX_EXPORT void nxAudioBusSetChannels(NxAudioSource* bus, uint32_t channels)
{
    static_cast<SoLoud::Bus*>(bus->handle)->setChannels(channels);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioBusEnableVisualization(NxAudioSource* bus, bool enabled)
{
    static_cast<SoLoud::Bus*>(bus->handle)->setVisualizationEnable(enabled);
}

//----------------------------------------------------------
NX_EXPORT const float* nxAudioBusCalcFFT(NxAudioSource* bus)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->calcFFT();
}

//----------------------------------------------------------
NX_EXPORT const float* nxAudioBusCurrentWaveData(NxAudioSource* bus)
{
    return static_cast<SoLoud::Bus*>(bus->handle)->getWave();
}
