#include <string>
#include "config.hpp"
#include "luavm.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include <SDL2/SDL.h>

extern "C" {

NX_EXPORT int testFunc()
{
    return 42;
}

}

int fatalError(const std::string& message, int retval = 1)
{
    Log::fatal(message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);

    return retval;
}

int main(int argc, char* argv[])
{
    Filesystem fs;
    LuaVM lua;
    int retval;

    // Initialize the filesystem
    {
        if (!fs.initialize((argc > 1) ? argv[0] : nullptr)) {
            return fatalError(Filesystem::getErrorMessage());
        }

        auto prefsDir = Filesystem::getPrefsDir();
        if (prefsDir.empty()) {
            return fatalError("Cannot retrieve the preferences directory");
        }

        Log::setLogFile(prefsDir + "/log.txt");

        auto baseDir = Filesystem::getBaseDir();
        if (baseDir.empty()) {
            return fatalError("Cannot retrieve the base directory of the application");
        }

        Log::info("Mounting preferences directory for writing: " + prefsDir);
        if (!Filesystem::setWriteDir(prefsDir)) {
            return fatalError("Cannot set writing directory: " + Filesystem::getErrorMessage());
        }

        Log::info("Mounting base directory for reading: " + baseDir);
        if (!Filesystem::mountDir(baseDir, "/")) {
            return fatalError("Cannot mount base directory: " + Filesystem::getErrorMessage());
        }

        Log::info("Mounting preferences directory for reading: " + prefsDir);
        if (!Filesystem::mountDir(prefsDir, "/")) {
            return fatalError("Cannot mount preferences directory: " +
                Filesystem::getErrorMessage());
        }

        Log::info("Mounting assets archive/directory for reaeding: " + baseDir + "assets");
        if (!Filesystem::mountDir(baseDir + "assets", "/assets") &&
            !Filesystem::mountArchive("assets.zip", "/assets")) {
            return fatalError("Cannot mount assets directory");
        }
    }

    // Run the lua code
    {
        std::string bootCode = R"(
            local ffi = require 'ffi'
            ffi.cdef[[
                int testFunc();
            ]]

            print('hello', 'world', ffi.C.testFunc())

            return 0
        )";

        if (!lua.initialize() || !lua.runCode("boot.lua", bootCode, retval)) {
            return fatalError(lua.getErrorMessage());
        }
    }

    return retval;
}
