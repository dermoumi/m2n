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
#include "luavm.hpp"

#include <luajit/lua.hpp>

//----------------------------------------------------------
LuaVM::~LuaVM()
{
    if (mState) lua_close(mState);
}

//----------------------------------------------------------
bool LuaVM::initialize()
{
    if (mState) return true;

    lua_State* state = luaL_newstate();
    if (!state) {
        mErrorMessage = "Could not create new Lua state";
        return false;
    }

    luaL_openlibs(state);

    mState = state;
    return true;
}

//----------------------------------------------------------
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
    if (lua_pcall(mState, 0, 1, 0) != 0) {
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

//----------------------------------------------------------
bool LuaVM::loadNxLibs()
{
    return loadNxLibs(mState);
}

//----------------------------------------------------------
std::string LuaVM::getErrorMessage() const
{
    return mErrorMessage;
}

//----------------------------------------------------------
bool LuaVM::loadNxLibs(lua_State* state)
{
    // Make sure we have a valid Lua state
    if (!state) return false;

    // Load data into a string?
    static std::string code(
        #include "../lua/nxlib.luainl"
    );

    // Load up the code into the Lua state
    if (luaL_loadbuffer(state, code.data(), code.size(), "nxlib.lua") != 0) return false;

    // Try to run the code
    if (lua_pcall(state, 0, 1, 0) != 0) return false;

    return true;
}

//==============================================================================
