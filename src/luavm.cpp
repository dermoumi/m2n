#include "luavm.hpp"
#include <luajit/lua.hpp>

LuaVM::~LuaVM()
{
    if (mState) lua_close(mState);
}

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

const std::string& LuaVM::getErrorMessage() const
{
    return mErrorMessage;
}
