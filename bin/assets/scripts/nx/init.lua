local ffi = require 'ffi'
local C = ffi.C
ffi.cdef [[
    void nxSysSleep(double s);
    double nxSysGetTime();
    void nxSysGetSDLError();
    const char* nxSysGetPlatform();
]]

local platform = ffi.string(C.nxSysGetPlatform()):lower()

return {
    sleep = C.nxSysSleep,
    getSystemTime = C.nxSysGetTime,
    getPlatform = function() return platform end
}