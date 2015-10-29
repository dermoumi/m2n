local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxWindow NxWindow;

    NxWindow* nxWindowGet();
    bool nxWindowCreate(const char*, int, int, bool);
    void nxWindowClose();
    void nxWindowDisplay();
    void nxWindowClear();
]]

local Window = {}

function Window.create(title, width, height, fullscreen)
    if C.nxWindowCreate(title, width, height, fullscreen) == nil then
        require 'nx'
        return nil, ffi.string(C.nxSysGetSDLError())
    end

    return true
end

function Window.close()
    C.nxWindowClose()
end

function Window.isOpen()
    return (C.nxWindowGet() ~= nil)
end

function Window.clear()
    C.nxWindowClear()
end

function Window.display()
    C.nxWindowDisplay()
end

return Window