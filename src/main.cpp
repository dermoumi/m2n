#include <string>
#include "config.hpp"
#include "luavm.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include <SDL2/SDL.h>
#include <physfs/physfs.h>

int fatalError(const std::string& message, int retval = 1)
{
    Log::fatal(message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);

    return retval;
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);

    Filesystem fs;

    // Initialize the filesystem
    if (!fs.initialize((argc > 1) ? argv[0] : nullptr)) {
        return fatalError(Filesystem::getErrorMessage());
    }

    auto prefsDir = Filesystem::getPrefsDir();
    if (prefsDir.empty()) {
        return fatalError("Cannot retrieve the preferences directory");
    }
    Log::setLogFile(prefsDir + "/log.txt");

    Log::info("Mounting preferences directory for writing: " + prefsDir);
    if (!Filesystem::setWriteDir(prefsDir)) {
        return fatalError("Cannot access preferences directory: " + Filesystem::getErrorMessage());
    }

    Log::info("Mounting preferences directory for reading: " + prefsDir);
    if (!Filesystem::mountDir(prefsDir, "/")) {
        return fatalError("Cannot access preferences directory: " +
            Filesystem::getErrorMessage());
    }

    #if defined(NX_SYSTEM_ANDROID)
        Log::info("Mounting assets directory for reading...");
        if (!Filesystem::mountAssetsDir(false)) {
            return fatalError("Cannot access assets directory");
        }

    #else
        auto baseDir = Filesystem::getBaseDir();
        if (baseDir.empty()) {
            return fatalError("Cannot retrieve the base directory of the application");
        }

        Log::info("Mounting base directory for reading: " + baseDir);
        if (!Filesystem::mountDir(baseDir, "/", false)) {
            return fatalError("Cannot access base directory: " + Filesystem::getErrorMessage());
        }

        Log::info("Mounting assets archive/directory for reading: " + baseDir + "assets");
        if (!Filesystem::mountAssetsDir(false) &&
            !Filesystem::mountArchive("assets.zip", "/assets", false)) {
            return fatalError("Cannot access assets directory");
        }

    #endif

    // Run the lua code
    {
        LuaVM lua;
        int retval;

        std::string bootCode (
            #include "boot.luainl"
        );

        if (!lua.initialize() || !lua.runCode("boot.lua", bootCode, retval)) {
            return fatalError(lua.getErrorMessage());
        }

        return retval;
    }
}
