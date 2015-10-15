local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxWindow NxWindow;

    NxWindow* nxWindowGet();
    bool nxWindowCreate(const char*, int, int, bool);
    void nxWindowClose();
]]

local Window = {}

function Window.create(title, width, height, fullscreen)
    local window = C.nxWindowCreate(title, width, height, fullscreen)
    if window == nil then
        require 'nx'
        return nil, ffi.string(C.nxSysGetSDLError())
    end
end

function Window.close()
    C.nxWindowClose()
end

return Window