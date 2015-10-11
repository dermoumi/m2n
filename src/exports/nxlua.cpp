#include "../config.hpp"
#include <luajit/lua.hpp>
#include <string>

extern "C"
{
    NX_EXPORT bool nxLuaLoadNxLibs(lua_State* state)
    {
        // Load data into a string?
        std::string code(
            #include "nxlib.luainl"
        );

        // Make sure we have a valid Lua state
        if (!state) return false;

        // Load up the code into the Lua state
        if (luaL_loadbuffer(state, code.data(), code.size(), "nxlib.lua") != 0) return false;

        // Try to run the code
        if (lua_pcall(state, 0, 1, 0) != 0) return false;

        return true;
    }

    NX_EXPORT void* nxLuaToCdata(lua_State* state, int index)
    {
        auto ptr = lua_topointer(state, index);
        return reinterpret_cast<void*>(*reinterpret_cast<const uintptr_t*>(ptr));
    }
}