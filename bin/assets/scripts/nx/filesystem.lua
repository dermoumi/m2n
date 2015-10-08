local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    const char* nxFsGetError();

    char** nxFsEnumerateFiles(const char*);
    void nxFsFreeList(char**);
    bool nxFsIsFile(const char*);
    bool nxFsIsDirectory(const char*);
]]

-- Main Filesystem functions ---------------------------------------------------
local function enumerateFiles(path)
    local files = C.nxFsEnumerateFiles(path)
    if files == nil then
        return nil, ffi.string(C.nxFsGetError())
    end

    local fileList = {}
    while files[#fileList] ~= nil do
        fileList[#fileList + 1] = ffi.string(files[#fileList])
    end

    C.nxFsFreeList(files)

    return fileList;
end

local function isDirectory(path)
    return C.nxFsIsDirectory(path)
end

local function isFile(path)
    return C.nxFsIsFile(path)
end

--------------------------------------------------------------------------------
return {
    enumerateFiles = enumerateFiles,
    isFile         = isFile,
    isDirectory    = isDirectory
}