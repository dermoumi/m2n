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
]]

local LuaVM = require 'nx.luavm'
local Thread = class 'Thread'

function Thread.static._fromCData(data)
    local thread = Thread:allocate()
    thread._handle = ffi.cast('NxThreadObj*', data)
    thread._vm = LuaVM._fromCData(thread._handle[0].state);
    return thread
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

    print(vm:_cdata())
    local handle = C.nxThreadCreate(vm:_cdata())
    if handle == nil then
        return nil, 'Cannot create a new thread'
    end

    self._vm = vm
    self._handle = ffi.gc(handle, C.nxThreadRelease)
end

function Thread:_cdata()
    return self._handle
end

function Thread:join()
    local ok = C.nxThreadWait(self._handle)
    if ok then
        local retCount = self._vm:getTop()
        print('retcount', retCount)
        return self._vm:pop(retCount, true)
    end

    -- If something's wrong
    return nil, self._vm:pop(1, true)
end

return Thread