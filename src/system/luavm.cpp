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

#include "luavm.hpp"

#include <luajit/lua.hpp>

LuaVM::~LuaVM()
{
    if (mState) lua_close(mState);
}

bool LuaVM::initialize(int argc, char** argv)
{
    if (mState) return true;

    lua_State* state = luaL_newstate();
    if (!state) {
        mErrorMessage = "Could not create new Lua state";
        return false;
    }

    // Push arguments
    lua_newtable(state);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(state, argv[i]);
        lua_rawseti(state, -2, i);
    }
    lua_setglobal(state, "arg");

    // Load NX library functions
    if (!loadNxLibs(state, &mErrorMessage)) {
        mErrorMessage = "Could not load Lua libraries: " + mErrorMessage;
        return false;
    }

    // Reserve the first slot for the debug function
    lua_settop(state, 0);
    lua_getglobal(state, "NX_Debug");

    mState = state;
    return true;
}

bool LuaVM::runCode(const std::string& filename, const std::string& code, int& retval)
{
    // Make sure we have a valid Lua state
    if (!mState) {
        mErrorMessage = "Cannot run code on invalid Lua state";
        return false;
    }

    // Load up the code into the Lua state
    if (luaL_loadbuffer(mState, code.data(), code.size(), filename.data()) != 0) {
        mErrorMessage = lua_tostring(mState, -1);
        return false;
    }

    // Try to run the code
    if (lua_pcall(mState, 0, 1, 1) != 0) {
        mErrorMessage = lua_tostring(mState, -1);
        return false;
    }

    // Try to retrieve the return value
    if (lua_isnumber(mState, -1)) {
        retval = lua_tonumber(mState, -1);
        return true;
    }
    else {
        mErrorMessage = "Invalid return value";
        return false;
    }
}

std::string LuaVM::getErrorMessage() const
{
    return mErrorMessage;
}

bool LuaVM::loadNxLibs(lua_State* state, std::string* err)
{
    // Make sure we have a valid Lua state
    if (!state) {
        if (err) *err = "Invalid state";
        return false;
    }

    // Lua libraries to load
    static const luaL_Reg lj_lib_load[] = {
        { "",              luaopen_base },
        { LUA_LOADLIBNAME, luaopen_package },
        { LUA_TABLIBNAME,  luaopen_table },
        // { LUA_IOLIBNAME,   luaopen_io },
        // { LUA_OSLIBNAME,   luaopen_os },
        { LUA_STRLIBNAME,  luaopen_string },
        { LUA_MATHLIBNAME, luaopen_math },
        { LUA_DBLIBNAME,   luaopen_debug },
        { LUA_BITLIBNAME,  luaopen_bit },
        // { LUA_JITLIBNAME,  luaopen_jit },
        { NULL,            NULL }
    };

    static const luaL_Reg lj_lib_preload[] = {
        { LUA_FFILIBNAME, luaopen_ffi },
        { NULL,           NULL }
    };

    // Attempt to load lua libs
    const luaL_Reg *lib;
    for (lib = lj_lib_load; lib->func; lib++) {
        lua_pushcfunction(state, lib->func);
        lua_pushstring(state, lib->name);

        if (lua_pcall(state, 1, 0, 0) != 0) {
            if (err) *err = lua_tolstring(state, -1, nullptr);
            return false;
        }
    }

    luaL_findtable(
        state, LUA_REGISTRYINDEX, "_PRELOAD", sizeof(lj_lib_preload)/sizeof(lj_lib_preload[0])-1
    );

    for (lib = lj_lib_preload; lib->func; lib++) {
        lua_pushcfunction(state, lib->func);
        lua_setfield(state, -2, lib->name);
    }
    lua_pop(state, 1);

    // Load custom nxLibs
    static std::string code(
        #include "../lua/nxlib.luainl"
    );

    // Load up the code into the Lua state
    if (luaL_loadbuffer(state, code.data(), code.size(), "nxlib.lua") != 0) return false;

    // Try to run the code
    if (lua_pcall(state, 0, 1, 0) != 0) {
        if (err) *err = lua_tolstring(state, -1, nullptr);
        return false;
    }

    return true;
}
