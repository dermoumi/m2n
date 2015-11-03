--[[----------------------------------------------------------------------------
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
--]]----------------------------------------------------------------------------

------------------------------------------------------------
-- ffi C declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct lua_State lua_State;
    typedef struct NxThreadObj {
        void* handle;
        lua_State* state;
        bool succeeded;
    } NxThreadObj;

    NxThreadObj* nxThreadCreate(lua_State*);
    void nxThreadRelease(NxThreadObj*);
    bool nxThreadWait(NxThreadObj*);
    bool nxThreadIsMain();
]]

------------------------------------------------------------
-- A class to create and use a thread
------------------------------------------------------------
local class = require 'nx.class'
local LuaVM = require 'nx.luavm'
local Thread = class 'nx.thread'

------------------------------------------------------------
function Thread.static._fromCData(data)
    local thread = Thread:allocate()
    thread._cdata = ffi.cast('NxThreadObj*', data)
    thread._vm = LuaVM._fromCData(thread._cdata[0].state);
    return thread
end

------------------------------------------------------------
function Thread.static.isMain()
    return C.nxThreadIsMain()
end

------------------------------------------------------------
function Thread:initialize(func, ...)
    if type(func) ~= 'function' then
        return nil, 'Callback is not a function'
    end

    local vm, err = LuaVM:new()
    if not vm then
        return nil, err
    end

    vm:push(func, ...)

    local handle = C.nxThreadCreate(vm._cdata)
    if handle == nil then
        return nil, 'Cannot create a new thread'
    end

    self._vm = vm
    self._cdata = ffi.gc(handle, C.nxThreadRelease)
end

------------------------------------------------------------
function Thread:join()
    local ok = C.nxThreadWait(self._cdata)
    if ok then
        local retCount = self._vm:getTop()
        return self._vm:pop(retCount, true)
    end

    -- If something's wrong
    return nil, self._vm:pop(1, true)
end

------------------------------------------------------------
return Thread