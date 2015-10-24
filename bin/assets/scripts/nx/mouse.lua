local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    bool nxMouseVisible(int);
    void nxMouseSetSystemCursor(int);
    int nxMouseGetCursor();
    void nxMouseSetPosition(int, int, bool);
    void nxMouseGetPosition(int*, bool);
    bool nxMouseSetRelativeMode(bool);
    bool nxMouseGetRelativeMode();
    bool nxMouseIsButtonDown(int);
    bool nxMouseSetGrab(bool);
    bool nxMouseIsGrabbed();
]]

--------------------------------------------------------------------------------
local Mouse = {}

local cursors = {
    [-2] = 'image',
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

function Mouse.setVisible(visible)
    return C.nxMouseVisible(visible and 1 or 0)
end

function Mouse.isVisible()
    return C.nxMouseVisible(-1)
end

function Mouse.setCursor(cursor, originX, originY)
    if type(cursor) == 'string' then
        if cursor == 'image' then
            return
        end

        for i = -1, #cursors do
            if cursors[i] == cursor then
                C.nxMouseSetSystemCursor(i)
                return
            end
        end
    else
        -- TODO: Set an image as cursor
    end
end

function Mouse.getCursor()
    return cursors[C.nxMouseGetCursor()]
end

function Mouse.setPosition(x, y)
    C.nxMouseSetPosition(x, y, false)
end

function Mouse.getPosition()
    local posPtr = ffi.new('int[2]')
    C.nxMouseGetPosition(posPtr, false)
    return tonumber(posPtr[0]), tonumber(posPtr[1])
end

function Mouse.setRelativeMode(relative)
    return C.nxMouseSetRelativeMode(relative)
end

function Mouse.getRelativeMode()
    return C.nxMouseGetRelativeMode()
end

function Mouse.isButtonDown(button)
    return C.nxMouseIsButtonDown(buttons[button])
end

function Mouse.grab(grabbed)
    -- return C.nxMouseSetGrab(grabbed)
    return false
end

function Mouse.isGrabbed()
    -- return C.nxMouseIsGrabbed()
    return false
end

return Mouse