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
-- Represents a font stack
------------------------------------------------------------
local Font = require 'nx.font'
local FontStack = Font:subclass('nx.fontstack')

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxFont NxFont;

    NxFont* nxFontStackNew();
    void nxFontStackAddFont(NxFont*, const NxFont*, bool);
    void nxFontStackAddStack(NxFont*, const NxFont*, bool);
]]

------------------------------------------------------------
function FontStack.static._fromCData(cdata)
    local stack = FontStack:allocate()
    stack._cdata = ffi.cast('NxFont*', cdata)
    stack._fonts = {}
    return stack
end

------------------------------------------------------------
function FontStack:initialize()
    local handle = C.nxFontStackNew()
    self._cdata = ffi.gc(handle, C.nxFontRelease)
    self._fonts = {}
end

------------------------------------------------------------
function FontStack:addFont(font, prepend)
    if font:isInstanceOf(FontStack) then
        for i, v in ipairs(font._fonts) do
            self._fonts[#self._fonts + 1] = v
        end
        C.nxFontStackAddStack(self._cdata, font._cdata, not not prepend)
    else
        self._fonts[#self._fonts] = font
        C.nxFontStackAddFont(self._cdata, font._cdata, not not prepend)
    end
end

------------------------------------------------------------
return FontStack
