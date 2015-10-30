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

ffi.cdef[[
    const char* nxFsGetError();

    char** nxFsEnumerateFiles(const char*);
    void nxFsFreeList(char**);
    bool nxFsIsFile(const char*);
    bool nxFsIsDirectory(const char*);
]]

------------------------------------------------------------
-- A collection of functions to interact with the FS
------------------------------------------------------------
local Filesystem = {}

------------------------------------------------------------
function Filesystem.enumerateFiles(path)
    -- Make sure the arguments are valid
    if type(path) ~= 'string' or path == '' then
        return {}, 'Invalid argument'
    end

    -- Retrieve files as C list
    local files = C.nxFsEnumerateFiles(path)
    if files == nil then
        return {}, ffi.string(C.nxFsGetError())
    end

    -- Convert it to a lua table
    local fileList = {}
    while files[#fileList] ~= nil do
        fileList[#fileList + 1] = ffi.string(files[#fileList])
    end

    -- Free the C list
    C.nxFsFreeList(files)

    return fileList;
end

------------------------------------------------------------
function Filesystem.isDirectory(path)
    -- Make sure the arguments are valid
    if type(path) ~= 'string' or path == '' then
        return false, 'Invalid argument'
    end

    return C.nxFsIsDirectory(path)
end

------------------------------------------------------------
function Filesystem.isFile(path)
    -- Make sure the arguments are valid
    if type(path) ~= 'string' or path == '' then
        return false, 'Invalid argument'
    end

    return C.nxFsIsFile(path)
end

--------------------------------------------------------------------------------
return Filesystem