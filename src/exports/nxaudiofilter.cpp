/*
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
*/

#include "../config.hpp"
#include "../audio/audio.hpp"

#include <soloud/soloud_filter.h>
#include <soloud/soloud_biquadresonantfilter.h>
#include <soloud/soloud_echofilter.h>
#include <soloud/soloud_lofifilter.h>
#include <soloud/soloud_flangerfilter.h>
#include <soloud/soloud_dcremovalfilter.h>
#include <soloud/soloud_bassboostfilter.h>

using NxAudioFilter = SoLoud::Filter;

NX_EXPORT void nxAudioFilterRelease(NxAudioFilter* filter)
{
    delete filter;
}

NX_EXPORT NxAudioFilter* nxAudioFilterBiquadResonantCreate()
{
    return new SoLoud::BiquadResonantFilter();
}

NX_EXPORT void NxAUdioFilterBiquadResonantSetParams(NxAudioFilter* filter, int type,
    float samplerate, float frequency, float resonance)
{
    static_cast<SoLoud::BiquadResonantFilter*>(filter)->setParams(
        type, samplerate, frequency, resonance
    );
}

NX_EXPORT NxAudioFilter* nxAudioFilterEchoCreate()
{
    return new SoLoud::EchoFilter();
}

NX_EXPORT void nxAudioFilterEchoSetParams(NxAudioFilter* echo, float delay, float decay,
    float filter)
{
    static_cast<SoLoud::EchoFilter*>(echo)->setParams(delay, decay, filter);
}

NX_EXPORT NxAudioFilter* nxAudioFilterLofiCreate()
{
    return new SoLoud::LofiFilter();
}

NX_EXPORT void nxAudioFilterLofiSetParams(NxAudioFilter* lofi, float samplerate, float bitdepth)
{
    static_cast<SoLoud::LofiFilter*>(lofi)->setParams(samplerate, bitdepth);
}

NX_EXPORT NxAudioFilter* nxAudioFilterFlangerCreate()
{
    return new SoLoud::FlangerFilter();
}

NX_EXPORT void nxAudioFilterFlangerSetParams(NxAudioFilter* flanger, float delay, float freq)
{
    static_cast<SoLoud::FlangerFilter*>(flanger)->setParams(delay, freq);
}

NX_EXPORT NxAudioFilter* nxAudioFilterDcRemovalCreate()
{
    return new SoLoud::DCRemovalFilter();
}

NX_EXPORT void nxAudioFilterDcRemovalSetParams(NxAudioFilter* dcRemoval, float length)
{
    static_cast<SoLoud::DCRemovalFilter*>(dcRemoval)->setParams(length);
}

NX_EXPORT NxAudioFilter* nxAudioFilterBassboostCreate()
{
    return new SoLoud::BassboostFilter();
}

NX_EXPORT void nxAudioFilterBassboostSetParams(NxAudioFilter* bassboost, float boost)
{
    static_cast<SoLoud::BassboostFilter*>(bassboost)->setParams(boost);
}
