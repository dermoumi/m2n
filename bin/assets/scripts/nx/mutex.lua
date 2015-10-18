local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxMutex NxMutex;

    NxMutex* nxMutexCreate();
    void nxMutexRelease(NxMutex*);
    void nxMutexLock(NxMutex*);
    bool nxMutexTryLock(NxMutex*);
    void nxMutexUnlock(NxMutex*);
]]

local class = require 'nx.class'
local Mutex = class 'nx.mutex'

function Mutex.static._fromCData(data)
    local mutex = Mutex:allocate()
    mutex._cdata = ffi.cast('NxMutex*', data)
    return mutex
end

function Mutex:initialize()
    self._cdata = ffi.gc(C.nxMutexCreate(), C.nxMutexRelease)
end

function Mutex:lock()
    C.nxMutexLock(self._cdata)
end

function Mutex:tryLock()
    return C.nxMutexTryLock(self._cdata)
end

function Mutex:unlock()
    C.nxMutexUnlock(self._cdata)
end

return Mutex