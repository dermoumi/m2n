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
    typedef struct NxWindow NxWindow;

    NxWindow* nxWindowGet();
    bool nxWindowCreate(const char*, int, int, bool);
    void nxWindowClose();
    void nxWindowDisplay();
    void nxWindowSetTitle(const char*);
]]

------------------------------------------------------------
-- A set of functions to manage the main window
------------------------------------------------------------
local Window = {}

------------------------------------------------------------
function Window.create(title, width, height, fullscreen)
    if C.nxWindowCreate(title, width, height, fullscreen) == nil then
        require 'nx' -- For the GetSDLError() C declaration
        return nil, ffi.string(C.nxSysGetSDLError())
    end

    return true
end

------------------------------------------------------------
function Window.close()
    C.nxWindowClose()
end

------------------------------------------------------------
function Window.isOpen()
    return (C.nxWindowGet() ~= nil)
end

------------------------------------------------------------
function Window.display()
    C.nxWindowDisplay()
end

------------------------------------------------------------
function Window.setTitle(title)
    C.nxWindowSetTitle(title)
end

------------------------------------------------------------
return Window