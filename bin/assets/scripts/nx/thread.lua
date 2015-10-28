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

local class = require 'nx.class'
local LuaVM = require 'nx.luavm'
local Thread = class 'nx.thread'

function Thread.static._fromCData(data)
    local thread = Thread:allocate()
    thread._cdata = ffi.cast('NxThreadObj*', data)
    thread._vm = LuaVM._fromCData(thread._cdata[0].state);
    return thread
end

function Thread.static.isMain()
    return C.nxThreadIsMain()
end

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

function Thread:join()
    local ok = C.nxThreadWait(self._cdata)
    if ok then
        local retCount = self._vm:getTop()
        print('retcount', retCount)
        return self._vm:pop(retCount, true)
    end

    -- If something's wrong
    return nil, self._vm:pop(1, true)
end

return Thread