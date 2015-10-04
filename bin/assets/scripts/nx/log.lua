local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    void nxLogVerbose(const char* message);
    void nxLogDebug(const char* message);
    void nxLogInfo(const char* message);
    void nxLogError(const char* message);
    void nxLogFatal(const char* message);
]]

local function makeFunc(func)
    return function(message)
        func(tostring(message))
    end
end

return {
    verbose = makeFunc(C.nxLogVerbose),
    debug   = makeFunc(C.nxLogDebug),
    info    = makeFunc(C.nxLogInfo),
    error   = makeFunc(C.nxLogError),
    fatal   = makeFunc(C.nxLogFatal)
}