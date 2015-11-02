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
    bool nxRendererInit();
    void nxRendererClear(uint8_t, uint8_t, uint8_t, uint8_t);
    void nxRendererBegin();
    void nxRendererFinish();
    void nxRendererSetupViewport(int, int, int, int);
    void nxRendererTestInit();
    void nxRendererTestRender();
    void nxRendererTestRelease();
]]

------------------------------------------------------------
-- A set of functions about on-screen rendering
local Renderer = {}

------------------------------------------------------------
function Renderer.init()
    return C.nxRendererInit();
end

------------------------------------------------------------
function Renderer.clear(r, g, b, a)
    C.nxRendererClear(r or 0, g or 0, b or 0, a or 255)
end

------------------------------------------------------------
function Renderer.begin()
    C.nxRendererBegin()
end

------------------------------------------------------------
function Renderer.finish()
    C.nxRendererFinish()
end

------------------------------------------------------------
function Renderer.setupViewport(x, y, w, h)
    C.nxRendererSetupViewport(x, y, w, h)
end

------------------------------------------------------------
function Renderer.testInit()
    C.nxRendererTestInit()
end

------------------------------------------------------------
function Renderer.testRender()
    C.nxRendererTestRender()
end

------------------------------------------------------------
function Renderer.testRelease()
    C.nxRendererTestRelease()
end

------------------------------------------------------------
return Renderer