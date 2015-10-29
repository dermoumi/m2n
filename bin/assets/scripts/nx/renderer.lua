local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    void nxRendererClear(uint8_t, uint8_t, uint8_t, uint8_t);
]]

local Renderer = {}

function Renderer.clear(r, g, b, a)
    C.nxRendererClear(r or 0, g or 0, b or 0, a or 255)
end

return Renderer