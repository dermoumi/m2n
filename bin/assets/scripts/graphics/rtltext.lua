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

local Entity2D    = require 'graphics.entity2d'
local ArrayBuffer = require 'graphics.arraybuffer'
local Text        = require 'graphics.text'

local RtlText = Text:subclass('graphics.rtltext')
-- RtlText:include(Entity2D)

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    NxText* nxRtlTextNew();
]]

function RtlText:initialize(str, font, charSize)
    Entity2D.initialize(self)

    self._cdata = ffi.gc(C.nxRtlTextNew(), C.nxTextRelease)

    self:setString(str or '')
        :setFont(font)
        :setSize(charSize or 30)
        
    self._style = 0
    self._vertices = ArrayBuffer:new()
end

return RtlText