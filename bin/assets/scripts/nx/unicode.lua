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

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    const uint32_t* nxUnicodeUtf8To32(const char*, uint32_t*);
    const char* nxUnicodeUtf32To8(const uint32_t*, uint32_t*);
]]

------------------------------------------------------------
local Unicode = {}

------------------------------------------------------------
function Unicode.utf8To32(str)
    local sizePtr = ffi.new('uint32_t[1]')
    local strPtr = C.nxUnicodeUtf8To32(str, sizePtr)

    local utf32 = {}
    while #utf32 <= sizePtr[0] do
        utf32[#utf32 + 1] = strPtr[#utf32]
    end

    return utf32
end

------------------------------------------------------------
function Unicode.utf32To8(str)
    -- Make sure the string ends with a 0
    if str[#str] ~= 0 then str[#str + 1] = 0 end

    -- Convert it to table
    str = ffi.new('uint32_t[?]', #str, str)

    local sizePtr = ffi.new('uint32_t[1]')
    local strPtr = C.nxUnicodeUtf32To8(str, sizePtr)
    return ffi.string(strPtr, sizePtr[0])
end

------------------------------------------------------------
return Unicode
