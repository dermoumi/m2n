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
struct NxAudioSource
{
    SoLoud::AudioSource* handle {nullptr};
    Audio::File* file {nullptr}; // Used if the audio source needs an active file
};

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxAudioSource* nxAudioSoundCreate()
{
    auto source = new NxAudioSource();
    source->handle = new SoLoud::Wav();
    return source;
}


//----------------------------------------------------------
NX_EXPORT NxAudioSource* nxAudioMusicCreate()
{
    auto source = new NxAudioSource();
    source->handle = new SoLoud::WavStream();
    return source;
}

//----------------------------------------------------------
NX_EXPORT void nxAudioRelease(NxAudioSource* source)
{
    delete source->handle;
    delete source->file;
    delete source;
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSoundOpenFile(NxAudioSource* source, const char* filename)
{
    Audio::File file;
    file.open(filename);
    static_cast<SoLoud::Wav*>(source->handle)->loadFile(&file);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioSoundOpenMemory(NxAudioSource* source, uint8_t* buffer, size_t size)
{
    static_cast<SoLoud::Wav*>(source->handle)->loadMem(buffer, size, true, false);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioMusicOpenFile(NxAudioSource* source, const char* filename)
{
    source->file = new Audio::File();
    source->file->open(filename);
    static_cast<SoLoud::WavStream*>(source->handle)->loadFile(source->file);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioMusicOpenMemory(NxAudioSource* source, uint8_t* buffer, size_t size)
{
    static_cast<SoLoud::WavStream*>(source->handle)->loadMem(buffer, size, true, false);
}

//----------------------------------------------------------
NX_EXPORT void nxAudioPlay(NxAudioSource* source)
{
    Audio::instance().play(*source->handle);
}
