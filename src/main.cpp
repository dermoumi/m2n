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

#include "config.hpp"

#include "system/filesystem.hpp"
#include "system/thread.hpp"
#include "system/luavm.hpp"
#include "system/log.hpp"

#include <physfs/physfs.h>
#include <SDL2/SDL.h>
#include <string>

#if _MSC_VER >= 1900
    // Quick fix for visual studio 2015's little problem with SDL 2.0.3
    extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#endif

extern "C" void nxWindowClose();

void SDLRelease()
{
    nxWindowClose();
    SDL_Quit();
}

int fatalError(const std::string& message, int retval = 1)
{
    Log::fatal(message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);

    return retval;
}

int main(int argc, char* argv[])
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
    atexit(SDLRelease);

    // Initialize the filesystem
    Filesystem fs;
    if (!fs.initialize((argc > 1) ? argv[0] : nullptr)) {
        return fatalError(Filesystem::getErrorMessage());
    }

    auto prefsDir = Filesystem::getPrefsDir();
    if (prefsDir.empty()) {
        return fatalError("Cannot retrieve the preferences directory");
    }

    // Set the logs file as soon as possible
    Log::setLogFile(prefsDir + "/log.txt");

    Log::info("Mounting preferences directory for writing: " + prefsDir);
    if (!Filesystem::setWriteDir(prefsDir)) {
        return fatalError("Cannot access preferences directory: " + Filesystem::getErrorMessage());
    }

    Log::info("Mounting preferences directory for reading: " + prefsDir);
    if (!Filesystem::mountDir(prefsDir, "/userdata")) {
        return fatalError("Cannot access preferences directory: " +
            Filesystem::getErrorMessage());
    }

    #if defined(NX_SYSTEM_ANDROID)
        Log::info("Mounting assets directory for reading...");
        if (!Filesystem::mountAssetsDir("/assets", false)) {
            return fatalError("Cannot access assets directory");
        }
    #else
        Log::info("Mounting root filesystem for reading: /");
        if (!Filesystem::mountDir("/", "/sysroot", false)) {
            return fatalError("Cannot access root directory: " + Filesystem::getErrorMessage());
        }

        auto baseDir = Filesystem::getBaseDir();
        if (baseDir.empty()) {
            return fatalError("Cannot retrieve the base directory of the application");
        }

        Log::info("Mounting base directory for reading: " + baseDir);
        if (!Filesystem::mountDir(baseDir, "/", false)) {
            return fatalError("Cannot access base directory: " + Filesystem::getErrorMessage());
        }

        Log::info("Mounting assets archive/directory for reading: " + baseDir + "assets");
        if (!Filesystem::mountAssetsDir("/assets", false) &&
            !Filesystem::mountArchive("assets.zip", "/assets", false)) {
            return fatalError("Cannot access assets directory");
        }
    #endif

    // A simple separator
    Log::info("----");

    // Set the current thread as the main thread
    Thread::setMain();

    // Enable joystick events
    SDL_JoystickEventState(1);
    SDL_GameControllerEventState(1);

    // Only enable text input when needed?
    SDL_StopTextInput();

    // Run the lua code
    LuaVM lua;
    if (!lua.initialize(argc, argv) || !lua.runCode("boot.lua", "return require 'main'")) {
        return fatalError(lua.getErrorMessage());
    }

    return 0;
}
