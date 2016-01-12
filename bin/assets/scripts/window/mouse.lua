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

local Mouse = {}

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    bool nxMouseVisible(int);
    void nxMouseSetSystemCursor(int);
    void nxMouseSetImageCursor(const void*, int, int, int, int);
    void nxMouseSetPosition(int, int, bool);
    void nxMouseGetPosition(int*, bool);
    bool nxMouseSetRelativeMode(bool);
    bool nxMouseGetRelativeMode();
    bool nxMouseIsButtonDown(int);
    bool nxMouseSetGrab(bool);
    bool nxMouseIsGrabbed();
]]

local fromCursor = {
    [-1] = 'default',
    [0] = 'arrow',
    'Ibeam',
    'wait',
    'crosshair',
    'waitarrow',
    'sizenwse',
    'sizenesw',
    'sizewe',
    'sizens',
    'sizeall',
    'no',
    'hand',
}
local toCursor = {}
for i, v in ipairs(fromCursor) do
    toCursor[v] = i
end

local buttons = {
    'left',
    'middle',
    'right',
    'xbutton1',
    'xbutton2',

    left = 1,
    middle = 2,
    right = 3,
    xbutton1 = 4,
    xbutton2 = 5
}
Mouse._btn = buttons

local currentCursor, imageCursor = 'default', nil

function Mouse.setVisible(visible)
    return C.nxMouseVisible(visible and 1 or 0)
end

function Mouse.isVisible()
    return C.nxMouseVisible(-1)
end

function Mouse.setCursor(cursor, originX, originY)
    if not cursor then cursor = 'default' end
    
    if type(cursor) == 'string' then
        C.nxMouseSetSystemCursor(toCursor[cursor] or -1)
        currentCursor, imageCursor = cursor, nil
    else
        local width, height = cursor:size()
        C.nxMouseSetImageCursor(cursor:data(), width, height, originX, originY)
        imageCursor = cursor
    end

    return Mouse
end

function Mouse.getCursor()
    return imageCursor or currentCursor
end

function Mouse.setPosition(x, y)
    C.nxMouseSetPosition(x, y, false)

    return Mouse
end

function Mouse.position()
    local posPtr = ffi.new('int[2]')
    C.nxMouseGetPosition(posPtr, false)

    return tonumber(posPtr[0]), tonumber(posPtr[1])
end

function Mouse.setRelativeMode(relative)
    return C.nxMouseSetRelativeMode(relative)
end

function Mouse.isRelativeMode()
    return C.nxMouseGetRelativeMode()
end

function Mouse.isButtonDown(button)
    return C.nxMouseIsButtonDown(buttons[button])
end

function Mouse.grab(grabbed)
    Log.warning('Grabbing mouse is not implemented yet')
    -- return C.nxMouseSetGrab(grabbed)
    return Mouse
end

function Mouse.isGrabbed()
    Log.warning('Grabbing mouse is not implemented yet')
    -- return C.nxMouseIsGrabbed()
    return false
end

return Mouse