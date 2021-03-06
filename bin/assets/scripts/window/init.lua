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

local System = require 'system'
local Image  = require 'graphics.image'

local Window = {}

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxWindow NxWindow;

    NxWindow* nxWindowGet();
    bool nxWindowCreate(const char*, int, int, int, int, bool, bool, bool, int, int, bool, int, int,
        int, int, int);
    void nxWindowClose();
    void nxWindowDisplay();
    void nxWindowGetFlags(int*);
    void nxWindowEnsureContext();
    bool nxWindowGetDesktopSize(int, int*);
    int nxWindowGetDisplayCount();
    const char* nxWindowGetDisplayName(int);
    int nxWindowGetFullscreen();
    bool nxWindowGetVisible();
    const int* nxWindowGetDisplayModes(int, size_t*);
    void nxWindowGetSize(int*);
    void nxWindowGetPosition(int*);
    const char* nxWindowGetTitle();
    void nxWindowMinimize();
    void nxWindowSetPosition(int, int);
    void nxWindowSetSize(int, int);
    void nxWindowSetTitle(const char*);
    void nxWindowSimpleMessageBox(const char*, const char*, uint32_t, bool);
    int nxWindowMessageBox(const char*, const char*, const char**, uint32_t, uint32_t, uint32_t,
        uint32_t, bool);
    void nxWindowGetDrawableSize(int*);
    const uint8_t* nxWindowGetIcon(unsigned int*);
    void nxWindowSetIcon(unsigned int, unsigned int, const uint8_t*);
]]

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
local ToFsType = {}
for i, v in pairs(FsType) do
    ToFsType[v] = i
end

local PosType = {
    undefined = -1,
    center = -2
}
local ToPosType = {}
for i, v in pairs(PosType) do
    ToPosType[v] = i
end

local windowWidth, windowHeight
local drawableWidth, drawableHeight
local hasFocus, hasMouseFocus

local lastTime
local totalElapsedTime, frameCount, currentFPS, elapsedTime = 0, 0, 0, 0
local originalFramerateLimit, framerateLimit = 0, 0

local function drawableSize()
    local sizePtr = ffi.new('int[2]')
    C.nxWindowGetDrawableSize(sizePtr)

    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

local function checkFlags(flags)
    flags = flags or {}

    if flags.fullscreen == nil     then flags.fullscreen = false end
    if flags.fullscreentype == nil then flags.fullscreentype = 'auto' end
    if flags.display == nil        then flags.display = 1 end
    if flags.vsync == nil          then flags.vsync = true end
    if flags.resizable == nil      then flags.resizable = false end
    if flags.borderless == nil     then flags.borderless = false end
    if flags.minwidth == nil       then flags.minwidth = 480 end
    if flags.minheight == nil      then flags.minheight = 320 end
    if flags.highdpi == nil        then flags.highdpi = false end
    if flags.refreshrate == nil    then flags.refreshrate = 0 end
    if flags.x == nil              then flags.x = 'undefined' end
    if flags.y == nil              then flags.y = 'undefined' end
    if flags.depthbits == nil      then flags.depthbits = 24 end
    if flags.stencilbits == nil    then flags.stencilbits = 8 end

    -- Windowed mode and fullscreen don't mix up well
    if not flags.fullscreen then flags.vsync = false end

    return flags
end

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
        flags.display,
        flags.vsync,
        flags.resizable,
        flags.borderless,
        flags.minwidth,
        flags.minheight,
        flags.highdpi,
        flags.refreshrate,
        posX,
        posY,
        flags.depthbits,
        flags.stencilbits
    )

    if window == nil then
        error('Unable to create window: ' .. ffi.string(C.nxSysGetSDLError()))
    end

    windowWidth, windowHeight     = width, height
    drawableWidth, drawableHeight = drawableSize()
    hasFocus, hasMouseFocus       = true, true

    framerateLimit   = flags.vsync and 0 or originalFramerateLimit
    lastTime         = System.time()

    -- Initial mouse focus is impossible with SDL < 2.0.4 (need to get global position)
    -- local Mouse = require 'window.mouse'
    -- local mX, mY = Mouse.getPosition()
    -- print(mX, mY, width, height)
    -- hasMouseFocus = mX >= 0 and mY >= 0 and mX < width and mY < height
end

function Window.close()
    C.nxWindowClose()
end

function Window.isOpen()
    return (C.nxWindowGet() ~= nil)
end

function Window.resetFrameTime()
    elapsedTime = 0
end

function Window.display()
    C.nxWindowDisplay()

    -- Calculating FPS every whole second
    totalElapsedTime = totalElapsedTime + elapsedTime
    frameCount       = frameCount + 1
    if totalElapsedTime > 1 then
        currentFPS = frameCount / totalElapsedTime
        totalElapsedTime = totalElapsedTime % 1
        frameCount = 0
    end

    -- Waiting out left time of the frame
    System.sleep(framerateLimit == 0 and 0 or lastTime - System.time() + framerateLimit)

    local currTime = System.time()
    elapsedTime = currTime - lastTime
    lastTime = currTime
end

function Window.setFramerateLimit(limit)
    framerateLimit = limit or 0
    originalFramerateLimit = framerateLimit

    return Window
end

function Window.framerateLimit()
    return framerateLimit
end

function Window.currentFPS()
    return math.floor(currentFPS + .5)
end

function Window.frameTime()
    return elapsedTime
end

function Window.flags()
    local flags = {}
    local flagsPtr = ffi.new('int[13]')

    C.nxWindowGetFlags(flagsPtr)

    flags.fullscreen     = tonumber(flagsPtr[0]) > 0
    flags.fullscreentype = ToFsType[tonumber(flagsPtr[0])] or 'auto'
    flags.display        = tonumber(flagsPtr[1])
    flags.vsync          = flagsPtr[2] == 1
    flags.resizable      = flagsPtr[3] == 1
    flags.borderless     = flagsPtr[4] == 1
    flags.minwidth       = tonumber(flagsPtr[5])
    flags.minheight      = tonumber(flagsPtr[6])
    flags.highdpi        = flagsPtr[7] == 1
    flags.refreshrate    = tonumber(flagsPtr[8])
    flags.x              = ToPosType[tonumber(flagsPtr[9])]  or tonumber(flagsPtr[9])
    flags.y              = ToPosType[tonumber(flagsPtr[10])] or tonumber(flagsPtr[10])
    flags.depthbits      = tonumber(flagsPtr[11])
    flags.stencilbits    = tonumber(flagsPtr[12])

    return flags
end

function Window.ensureContext()
    return C.nxWindowEnsureContext() ~= nil
end

function Window.size(drawableSize)
    if drawableSize then
        return drawableWidth, drawableHeight
    end

    return windowWidth, windowHeight
end

function Window.position()
    local posPtr = ffi.new('int[2]')
    C.nxWindowGetPosition(posPtr)
    return tonumber(posPtr[0]), tonumber(posPtr[1])
end

function Window.title()
    return ffi.string(C.nxWindowGetTitle())
end

function Window.setPosition(x, y)
    C.nxWindowSetPosition(x, y)

    return Window
end

function Window.setTitle(title)
    C.nxWindowSetTitle(title)

    return Window
end

function Window.displayCount()
    return C.nxWindowGetDisplayCount()
end

function Window.desktopSize(display)
    local sizePtr = ffi.new('int[2]')
    if not C.nxWindowGetDesktopSize(display or 1, sizePtr) then return end

    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

function Window.displayName(display)
    local name = C.nxWindowGetDisplayName(display or 1)
    if name == nil then return '' end

    return ffi.string(name)
end

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

function Window.isVisible()
    return C.nxWindowGetVisible()
end

function Window.hasFocus()
    return hasFocus
end

function Window.hasMouseFocus()
    return hasMouseFocus
end

function Window.displayModes(display)
    local countPtr = ffi.new('size_t[1]')
    local modesPtr = C.nxWindowGetDisplayModes(display or 1, countPtr)

    local modes = {}
    for i = 1, tonumber(countPtr[0]) / 2 do
        modes[i] = {
            width  = tonumber(modesPtr[(i - 1) * 2]),
            height = tonumber(modesPtr[(i - 1) * 2 + 1])
        }
    end

    return modes
end

function Window.minimize()
    C.nxWindowMinimize()

    return Window
end

function Window.showMessageBox(title, message, a, b, c)
    if type(a) == 'table' then
        if c == nil then c = true end

        for i, v in ipairs(a) do
            a[i] = ffi.new('char[?]', #v, v)
        end

        local entries = ffi.cast('const char**', ffi.new('char*[?]', #a, a))
        return C.nxWindowMessageBox(
            title, message, entries, #a, a.accept or 0, a.cancel or 0,
            MsgBoxType[b] or MsgBoxType.error, c
        )
    else
        if b == nil then b = true end
        C.nxWindowSimpleMessageBox(
            title, message, MsgBoxType[a] or MsgBoxType.error, b
        )
    end
end

function Window.toPixel(x, y)
    return x * drawableWidth / windowWidth, y * drawableHeight / windowHeight
end

function Window.fromPixel(x, y)
    return x * windowWidth / drawableWidth, y * windowHeight / drawableHeight
end

function Window.setIcon(icon)
    if type(icon) == 'string' then icon = Image:new(icon) end

    local w, h = icon:size()
    C.nxWindowSetIcon(w, h, icon:data())
    return true
end

function Window.getIcon()
    local sizePtr = ffi.new('unsigned int[2]')
    local dataPtr = C.nxWindowGetIcon(sizePtr)

    return Image.create(tonumber(sizePtr[0]), tonumber(sizePtr[1]), dataPtr)
end

function Window.__resize(w, h)
    windowWidth,   windowHeight   = w, h
    drawableWidth, drawableHeight = drawableSize()
end

function Window.__focus(focus)
    hasFocus = focus
end

function Window.__mouseFocus(focus)
    hasMouseFocus = focus
end

return Window
