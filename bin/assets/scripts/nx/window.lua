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
    bool nxWindowCreate(const char*, int, int, int, bool, bool, bool, int, int, bool, int, int,
        int, unsigned int, unsigned int, unsigned int);
    void nxWindowClose();
    void nxWindowDisplay();
    void nxWindowEnsureContext();
    void nxWindowReleaseContext();
    bool nxWindowGetDesktopSize(int, int*);
    int nxWindowGetDisplayCount();
    const char* nxWindowGetDisplayName(int);
    int nxWindowGetFullscreen();
    const int* nxWindowGetDisplayModes(int, size_t*);
    void nxWindowGetSize(int*);
    void nxWindowGetPosition(int*);
    const char* nxWindowGetTitle();
    void nxWindowMinimize();
    void nxWindowSetPosition(int, int);
    void nxWindowSetSize(int, int);
    void nxWindowSetTitle(const char*);
    void nxWindowSimpleMessageBox(const char*, const char*, uint32_t, bool);
    void nxWindowGetDrawableSize(int*);
]]

------------------------------------------------------------
-- Constants
------------------------------------------------------------
local MsgBoxType = {
    ['error']   = 16,
    ['warning'] = 32,
    ['info']    = 64
}

local FsType = {
    normal = 1,
    desktop = 2,
    auto = 3
}

local PosType = {
    undefined = -1,
    center = -2
}

------------------------------------------------------------
-- A set of functions to manage the main window
------------------------------------------------------------
local Window = {}
local windowWidth, windowHeight
local drawableWidth, drawableHeight

------------------------------------------------------------
local function getDrawableSize()
    local sizePtr = ffi.new('int[2]')
    C.nxWindowGetDrawableSize(sizePtr)
    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

------------------------------------------------------------
local function checkFlags(flags)
    flags = flags or {}

    if flags.fullscreen == nil     then flags.fullscreen = false end
    if flags.fullscreentype == nil then flags.fullscreentype = 'auto' end
    if flags.vsync == nil          then flags.vsync = true end
    if flags.resizable == nil      then flags.resizable = false end
    if flags.borderless == nil     then flags.borderless = false end
    if flags.minwidth == nil       then flags.minwidth = 480 end
    if flags.minheight == nil      then flags.minheight = 320 end
    if flags.highdpi == nil        then flags.highdpi = false end
    if flags.refreshrate == nil    then flags.refreshrate = 0 end
    if flags.x == nil              then flags.x = 'undefined' end
    if flags.y == nil              then flags.y = 'undefined' end
    if flags.depth == nil          then flags.depth = 0 end
    if flags.stencil == nil        then flags.stencil = 0 end
    if flags.msaa == nil           then flags.msaa = 0 end

    return flags
end

------------------------------------------------------------
function Window.create(title, width, height, flags)
    flags = checkFlags(flags)

    local fullscreen = flags.fullscreen and FsType[flags.fullscreentype] or false
    local posX = PosType[flags.x] or flags.x
    local posY = PosType[flags.y] or flags.y

    local window = C.nxWindowCreate(
        title,
        width,
        height,
        fullscreen,
        flags.vsync,
        flags.resizable,
        flags.borderless,
        flags.minwidth,
        flags.minheight,
        flags.highdpi,
        flags.refreshrate,
        posX,
        posY,
        flags.depth,
        flags.stencil,
        flags.msaa
    )

    if window == nil then
        require 'nx' -- For the GetSDLError() C declaration
        return nil, ffi.string(C.nxSysGetSDLError())
    end

    windowWidth, windowHeight     = width, height
    drawableWidth, drawableHeight = getDrawableSize()
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
function Window.ensureContext()
    return C.nxWindowEnsureContext() ~= nil
end

------------------------------------------------------------
function Window.size()
    return windowWidth, windowHeight
end

------------------------------------------------------------
function Window.position()
    local posPtr = ffi.new('int[2]')
    C.nxWindowGetPosition(posPtr)
    return tonumber(posPtr[0]), tonumber(posPtr[1])
end

------------------------------------------------------------
function Window.title()
    return ffi.string(C.nxWindowGetTitle())
end

------------------------------------------------------------
function Window.setPosition(x, y)
    C.nxWindowSetPosition(x, y)
end

------------------------------------------------------------
function Window.setTitle(title)
    C.nxWindowSetTitle(title)
end

------------------------------------------------------------
function Window.displayCount()
    return C.nxWindowGetDisplayCount()
end

------------------------------------------------------------
function Window.desktopSize(display)
    display = display or 1

    local sizePtr = ffi.new('int[2]')
    if not C.nxWindowGetDesktopSize(display, sizePtr) then return end
    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

------------------------------------------------------------
function Window.displayName(display)
    display = display or 1

    local name = C.nxWindowGetDisplayName(display)
    if name == nil then return '' end

    return ffi.string(name)
end

------------------------------------------------------------
function Window.isFullscreen()
    local fs = C.nxWindowGetFullscreen()
    if fs == 2 then
        return true, 'desktop'
    elseif fs == 1 then
        return true, 'normal'
    else
        return false
    end
end

------------------------------------------------------------
function Window.displayModes(display)
    display = display or 1

    local countPtr = ffi.new('size_t[1]')
    local modesPtr = C.nxWindowGetDisplayModes(display, countPtr)

    local modes = {}
    for i = 1, tonumber(countPtr[0]) / 2 do
        modes[i] = {
            width  = tonumber(modesPtr[(i - 1) * 2]),
            height = tonumber(modesPtr[(i - 1) * 2 + 1])
        }
    end

    return modes
end

------------------------------------------------------------
function Window.minimize()
    C.nxWindowMinimize()
end

------------------------------------------------------------
function Window.showMessageBox(title, message, type, attach)
    if not attach then attach = false end

    C.nxWindowSimpleMessageBox(title, message, MsgBoxType[type] or MsgBoxType.error, attach)
end

------------------------------------------------------------
function Window.toPixel(x, y)
    return x * drawableWidth / windowWidth, y * drawableHeight / windowHeight
end

------------------------------------------------------------
function Window.fromPixel(x, y)
    return x * windowWidth / drawableWidth, y * windowHeight / drawableHeight
end

------------------------------------------------------------
function Window._resize(w, h)
    windowWidth,   windowHeight   = w, h
    drawableWidth, drawableHeight = getDrawableSize()
end

------------------------------------------------------------
return Window