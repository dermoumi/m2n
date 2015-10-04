#pragma once

// Defining platforms
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

// Portable Export/Import macros
#if defined(NX_SYSTEM_WINDOWS) || defined(NX_SYSTEM_WINCE)
    #define NX_EXPORT __declspec(dllexport)
    #define NX_IMPORT __declspec(dllimport)
    #define NX_HIDDEN
    
    // Turn off annoying C4251 warning on Visual C++ compilers
    #ifdef _MSC_VER
        #pragma warning(disable: 4251)
    #endif

#elif defined(NX_SYSTEM_ANDROID)
    #include <jni.h>
    #define NX_EXPORT JNIEXPORT
    #define NX_IMPORT JNIIMPORT
    #define NX_HIDDEN

#elif __GNUC__ >= 4
    #define NX_EXPORT __attribute__ ((__visibility__ ("default")))
    #define NX_IMPORT __attribute__ ((__visibility__ ("default")))
    #define NX_HIDDEN __attribute__ ((__visibility__ ("hidden")))

#else
    #define NX_EXPORT
    #define NX_IMPORT
    #define NX_HIDDEN
#endif

#include <cstdint> // Integer types
#include <cstddef> // nullptr_t and size_t

constexpr char GAME_FULLTITLE[]  = "Monsters of 2nd Night";
constexpr char GAME_SHORTTITLE[] = "m2n";
constexpr char GAME_ORGNAME[]    = "nxsie";
