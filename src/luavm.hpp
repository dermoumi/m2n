#pragma once
#include <string>
#include "config.hpp"

struct lua_State;
class NX_HIDDEN LuaVM
{
public:
    LuaVM() = default;
    ~LuaVM();

    bool initialize();
    bool runCode(const std::string& filename, const std::string& code, int& retval);

    const std::string& getErrorMessage() const;

private:
    lua_State* mState {nullptr};
    std::string mErrorMessage;
};