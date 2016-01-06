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

local Font = require 'graphics.font'

local FontStack = Font:subclass('graphics.fontstack')

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxFont NxFont;

    NxFont* nxFontStackNew();
    void nxFontStackAddFont(NxFont*, const NxFont*, bool);
    void nxFontStackAddStack(NxFont*, const NxFont*, bool);
]]

function FontStack.static.factory(filename)
    local fonts = loadfile(filename)
    if fonts then 
        fonts = fonts()
    else
        return {
            funcs = {proc = function() return false end}
        }
    end

    local dependencies = {}
    for i, font in ipairs(fonts) do
        dependencies[font] = false
    end

    return {
        deps = dependencies,
        funcs = {
            {
                proc = function(fontStack, filename, ...)
                    for i, font in ipairs({...}) do
                        fontStack:addFont(font)
                    end
                end,
                threaded = false,
                deps = fonts
            }
        }
    }
end

function FontStack:initialize()
    local handle = C.nxFontStackNew()
    self._cdata = ffi.gc(handle, C.nxFontRelease)
    self._fonts = {}
end

function FontStack:addFont(font, prepend)
    if self._cdata ~= nil then
        if font:isInstanceOf(FontStack) then
            -- A font stack, append its child fonts instead
            for i, v in ipairs(font._fonts) do
                self._fonts[#self._fonts + 1] = v
            end
            C.nxFontStackAddStack(self._cdata, font._cdata, not not prepend)
        else
            -- A single font
            self._fonts[#self._fonts] = font
            C.nxFontStackAddFont(self._cdata, font._cdata, not not prepend)
        end
    end

    return self
end

return FontStack