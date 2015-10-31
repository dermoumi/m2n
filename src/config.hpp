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
#pragma once

//----------------------------------------------------------
// Defining platforms
//----------------------------------------------------------
#if defined(WINCE)
    #define NX_SYSTEM_WINCE
    #warning Windows CE support is incomplete and untested

#elif defined(WIN32) || defined(_WINDOWS) || defined(_WIN32)
    #define NX_SYSTEM_WINDOWS

#elif defined(__APPLE__) || defined(__APPLE_CC__)
    #include <TargetConditionals.h>

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define NX_SYSTEM_IOS
        #warning iOS support is incomplete and untested

    #elif TARGET_OS_MAC
        #define NX_SYSTEM_MACOS
        #warning Mac OS support is incomplete and untested

    #else
        #error This Apple operating system is not supported
    #endif

#elif defined(__unix__)
    #if defined(__ANDROID__)
        #define NX_SYSTEM_ANDROID

    #elif defined(__linux__)
        #define NX_SYSTEM_LINUX

    #elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
        #define NX_SYSTEM_FREEBSD
        #warning FreeBSD support is incomplete and untested
    
    #else
        #error This UNIX operating system is not supported
    #endif

#else
    #error This operating system is not supported
#endif

//----------------------------------------------------------
// Portable Export/Import macros
//----------------------------------------------------------
#if defined(NX_SYSTEM_WINDOWS) || defined(NX_SYSTEM_WINCE)
    #define NX_EXPORT extern "C" __declspec(dllexport)
    #define NX_IMPORT __declspec(dllimport)
    #define NX_HIDDEN
    
    // Turn off annoying C4251 warning on Visual C++ compilers
    #ifdef _MSC_VER
        #pragma warning(disable: 4251)
    #endif

#elif defined(NX_SYSTEM_ANDROID)
    #include <jni.h>
    #define NX_EXPORT extern "C" JNIEXPORT
    #define NX_IMPORT JNIIMPORT
    #define NX_HIDDEN

#elif __GNUC__ >= 4
    #define NX_EXPORT extern "C" __attribute__ ((__visibility__ ("default")))
    #define NX_IMPORT __attribute__ ((__visibility__ ("default")))
    #define NX_HIDDEN __attribute__ ((__visibility__ ("hidden")))

#else
    #define NX_EXPORT extern "C"
    #define NX_IMPORT
    #define NX_HIDDEN
#endif

//----------------------------------------------------------
// General include
//----------------------------------------------------------
#include <cstdint> // Integer types
#include <cstddef> // nullptr_t and size_t

//----------------------------------------------------------
// Constants
//----------------------------------------------------------
constexpr char GAME_FULLTITLE[]  = "Monsters of 2nd Night";
constexpr char GAME_SHORTTITLE[] = "m2n";
constexpr char GAME_ORGNAME[]    = "nxsie";

#if defined(NX_OPENGL_ES)
    constexpr unsigned int NX_GL_MAJOR = 2u;
    constexpr unsigned int NX_GL_MINOR = 0u;
#else
    constexpr unsigned int NX_GL_MAJOR = 2u;
    constexpr unsigned int NX_GL_MINOR = 1u;
#endif
    
//==============================================================================
