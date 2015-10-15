local ffi = require 'ffi'
local C = ffi.C
ffi.cdef [[
    void nxSysSleep(double s);
    double nxSysGetTime();
    void nxSysGetSDLError();
]]

return {
    sleep = C.nxSysSleep,
    getSystemTime = C.nxSysGetTime
}