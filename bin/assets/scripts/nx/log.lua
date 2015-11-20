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
    void nxLogVerbose(const char* message);
    void nxLogDebug(const char* message);
    void nxLogWarning(const char* message);
    void nxLogInfo(const char* message);
    void nxLogError(const char* message);
    void nxLogFatal(const char* message);
]]

------------------------------------------------------------
-- A set of functions used for logging
------------------------------------------------------------
local Log = {}

------------------------------------------------------------
function Log.verbose(message)
    C.nxLogVerbose(message)
end

------------------------------------------------------------
function Log.debug(message)
    C.nxLogDebug(message)
end

------------------------------------------------------------
function Log.warning(message)
    C.nxLogWarning(message)
end

------------------------------------------------------------
function Log.info(message)
    C.nxLogInfo(message)
end

------------------------------------------------------------
function Log.error(message)
    C.nxLogError(message)
end

------------------------------------------------------------
function Log.fatal(message)
    C.nxLogFatal(message)
end

------------------------------------------------------------
return Log