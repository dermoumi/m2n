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

local class = require 'nx.class'
local Log   = require 'nx.util.log'
local LuaVM = require 'nx.system.luavm'

local Thread = class 'nx.system.thread'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct lua_State lua_State;
    typedef struct NxThreadObj NxThreadObj;

    NxThreadObj* nxThreadCreate(lua_State*);
    void nxThreadRelease(NxThreadObj*);
    bool nxThreadWait(NxThreadObj*);
    void nxThreadDetach(NxThreadObj*);
    bool nxThreadIsMain();
]]

------------------------------------------------------------
function Thread.static.isMain()
    return C.nxThreadIsMain()
end

------------------------------------------------------------
function Thread:initialize(func, ...)
    self._vm = LuaVM:new()
    self._vm:push(func, ...)

    local handle = C.nxThreadCreate(self._vm._cdata)
    if handle == nil then
        Log.warning('Unable to create a new thread')
        self._vm:release()
        self._vm = nil
    end

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
function Thread:detach()
    C.nxThreadDetach(ffi.gc(self._cdata, nil))
    ffi.gc(self._vm._cdata, nil)
end

------------------------------------------------------------
function Thread:release()
    if self._cdata == nil then return end
    C.nxThreadRelease(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
return Thread