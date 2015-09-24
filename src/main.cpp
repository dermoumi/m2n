#include <SDL2/SDL.h>
#include <string>
#include <stdexcept>
#include "config.hpp"
#include "luavm.hpp"

extern "C" {

NX_EXPORT int testFunc()
{
    return 42;
}

}

int fatalError(const std::string& message, int retval = 1)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, message.data());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);

    return retval;
}

int main(int, char**)
{
    LuaVM lua;
    int retval;

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
