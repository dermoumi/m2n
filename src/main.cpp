#include <SDL2/SDL.h>
#include <lua.hpp>
#include <string>
#include <stdexcept>
#include "config.hpp"

extern "C" {

NX_EXPORT int testFunc()
{
    return 42;
}

}

void runLuaString(lua_State* state, const std::string& filename, const std::string& buffer)
{
    // Try to load the code into the lua state
    if (luaL_loadbuffer(state, buffer.data(), buffer.size(), filename.data()) != 0) {
        const char* luaErr = lua_tostring(state, -1);
        throw std::runtime_error(std::string("Error loading Lua boot script: ") + luaErr);
    }

    // Try to run the code
    if (lua_pcall(state, 0, 1, 0) != 0) {
        throw std::runtime_error(std::string("Lua script error: ") + lua_tostring(state, -1));
    }
}

int main(int, char**)
{
    // Run the lua code
    try {
        lua_State* state = luaL_newstate();
        if (!state) throw std::runtime_error("Could not create a Lua State. Aborting...");

        luaL_openlibs(state);
 
        std::string bootCode = R"(
            local ffi = require 'ffi'
            ffi.cdef[[
                int testFunc();
            ]]

            print('hello', 'world', ffi.C.testFunc())

            return true;
        )";

        runLuaString(state, "boot.lua", bootCode);
    }
    catch (const std::exception& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), nullptr);

        return 1;
    }

    return 1;
}
