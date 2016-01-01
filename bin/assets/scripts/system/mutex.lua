--[[
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
--]]

local class = require 'class'

local Mutex = class 'system.mutex'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxMutex NxMutex;

    NxMutex* nxMutexCreate();
    void nxMutexRelease(NxMutex*);
    bool nxMutexTryLock(NxMutex*);
    void nxMutexLock(NxMutex*);
    void nxMutexUnlock(NxMutex*);
]]

function Mutex:initialize()
    self._cdata = ffi.gc(C.nxMutexCreate(), C.nxMutexRelease)
end

function Mutex:tryLock()
    if self._cdata == nil then return end

    return C.nxMutexTryLock(self._cdata)
end

function Mutex:lock()
    if self._cdata == nil then return self end

    C.nxMutexLock(self._cdata)
    return self
end

function Mutex:unlock()
    if self._cdata == nil then return self end

    C.nxMutexUnlock(self._cdata)
    return self
end

return Mutex