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

class LuaState
{
public:
    ~LuaState()
    {
        if (state) lua_close(state);
    }

    lua_State* state {nullptr};
};

int fatalError(const std::string& message, int retval = 1)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, message.data());
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);

    return retval;
}

int main(int, char**)
{
    LuaState lua;

    // Run the lua code
    {
        lua.state = luaL_newstate();
        if (!lua.state) return fatalError("Could not create a Lua State. Aborting...");

        luaL_openlibs(lua.state);
 
        std::string bootCode = R"(
            local ffi = require 'ffi'
            ffi.cdef[[
                int testFunc();
            ]]

            print('hello', 'world', ffi.C.testFunc())

            return true;
        )";

        // Try to load the code into the lua state
        if (luaL_loadbuffer(lua.state, bootCode.data(), bootCode.size(), "boot.lua") != 0) {
            const char* luaErr = lua_tostring(lua.state, -1);
            return fatalError(std::string("Error loading Lua boot script: ") + luaErr);
        }

        // Try to run the code
        if (lua_pcall(lua.state, 0, 1, 0) != 0) {
            return fatalError(std::string("Lua script error: ") + lua_tostring(lua.state, -1));
        }
    }

    return 1;
}
