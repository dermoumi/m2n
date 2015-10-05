local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    const char* nxFsGetError();

    char** nxFsEnumerateFiles(const char*);
    void nxFsFreeList(char**);
    bool nxFsIsFile(const char* path);
    bool nxFsIsDirectory(const char* path);
]]

-- Main Filesystem functions ---------------------------------------------------
local function enumerateFiles(path)
    local files = C.nxFsEnumerateFiles(path)
    if files == nil then
        return nil, ffi.string(nxFsGetError())
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

-- Shared methods between read and write files ---------------------------------
local function close(handle)

end

local function length(handle)

end

local function tell(handle)

end

local function seek(handle, position)

end

--------------------------------------------------------------------------------
return {
    enumerateFiles = enumerateFiles,
    isDirectory = isDirectory,
    isFile = isFile
}