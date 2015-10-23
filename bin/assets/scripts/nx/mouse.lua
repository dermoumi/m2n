local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    bool nxMouseCursorVisible(int);
    void nxMouseSetSystemCursor(int);
    int nxMouseGetCursor();
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
    'xbutton2'
}
Mouse._btn = buttons

function Mouse.setCursorVisible(visible)
    return C.nxMouseCursorVisible(visible and 1 or 0)
end

function Mouse.isCursorVisible()
    return C.nxMouseCursorVisible(-1)
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

return Mouse