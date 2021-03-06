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

local class = require 'class'
local Log   = require 'util.log'

local LuaVM = class 'system.luavm'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct lua_State lua_State;

    bool nxLuaLoadNxLibs(lua_State*);
    void* nxLuaToCdata(lua_State*, int);

    lua_State* luaL_newstate();
    void lua_close(lua_State*);

    int lua_gettop(lua_State*);
    void lua_settop(lua_State*, int);

    void lua_createtable(lua_State*, int, int);
    int lua_next(lua_State*, int);

    void lua_getfield(lua_State*, int, const char*);
    void lua_rawset(lua_State*, int);

    int lua_pcall(lua_State*, int, int, int);

    void lua_pushnil(lua_State*);
    void lua_pushnumber(lua_State*, double);
    void lua_pushstring(lua_State*, const char*);
    void lua_pushlstring(lua_State*, const char*, size_t);
    void lua_pushboolean(lua_State*, int);
    void lua_pushlightuserdata(lua_State*, void*);
    void lua_pushvalue(lua_State*, int);

    double lua_tonumber(lua_State*, int);
    const char* lua_tolstring(lua_State*, int, size_t*);
    int lua_toboolean(lua_State*, int);
    void* lua_touserdata(lua_State*, int);

    const char* lua_typename(lua_State*, int);
    int lua_type(lua_State*, int);
]]

-- Helpers
local pushers = {}
local retrievers = {}

-- Returns the typename of the value at the given index of lua_State s
local function typeof(s, i)
    return ffi.string(C.lua_typename(s, C.lua_type(s, i)))
end

-- Returns a CPointer to the given data, no-op if already a pointer
local function toCPointer(cdata)
    -- Extract C-Type
    local ct = string.match(tostring(ffi.typeof(cdata)), 'ctype<(.+)>')
    if not ct then return nil end

    -- Check whether it's already a pointer
    if string.match(ct, '.+%*$') then
        -- type* pointer, no-op
        return cdata, ct, false
    elseif string.match(ct, '.+%]$') then
        -- type[n], make it into a generic type* pointer
        return cdata, string.match(ct, '(.+)%[') .. '*', false
    end

    -- Convert to pointer
    return ffi.new(ct .. '[1]', {cdata}), ct .. '   *', true
end

-- Defined later on
local nxClassPusher

-- Pushes the table t into the lua_State s
local function tablePusher(s, t)
    if type(t) == 'table' and t.class and t.class.name then
        nxClassPusher(s, t)
        return
    end

    C.lua_createtable(s, #t, 0)
    local top = C.lua_gettop(s)

    for k, v in pairs(t) do
        local keyPusher = pushers[type(k)]
        if keyPusher then
            local valPusher = pushers[type(v)]
            if valPusher then
                keyPusher(s, k)
                valPusher(s, v)
                C.lua_rawset(s, top)
            else
                error('LuaVM Table pusher: value type ' .. type(v) .. ' for key ' .. tostring(k) ..
                    ' is not supported')
            end
        else
            error('LuaVM Table pusher: key type ' .. type(v) .. ' is not supported.')
        end
    end
end

-- Pushes the function f into the lua_State s. f must be void of upvalues
local function funcPusher(s, f)
    local dump = string.dump(f)

    C.lua_getfield(s, -10002, 'loadstring')
    C.lua_pushlstring(s, dump, #dump)
    if C.lua_pcall(s, 1, 2, 0) ~= 0 then
        error('LuaVM Function pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
    end

    -- Remove the second return value in case there were no errors
    C.lua_settop(s, -2)
end

-- Pushes the object into the lua_State s
nxClassPusher = function(s, o)
    local table = {}

    for i, v in pairs(o) do
        if i ~= 'class' then
            table[i] = v
        end
    end

    -- Preload the class before pushing to make sure cdata attributes are defined with ffi
    C.lua_getfield(s, -10002, 'NX_ClassLoader')
    C.lua_pushstring(s, tostring(o.class.name))
    if C.lua_pcall(s, 1, 0, 0) ~= 0 then
        error('LuaVM nxLib Class Object pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
    end

    C.lua_getfield(s, -10002, 'NX_ClassObject')
    tablePusher(s, table)
    C.lua_pushstring(s, tostring(o.class.name))

    if C.lua_pcall(s, 2, 1, 0) ~= 0 then
        error('LuaVM nxLib Class Object pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
    end
end

-- Pushes c data into the lua_State s
local function cdataPusher(s, o)
    local ptr, type, dereference = toCPointer(o)
    if not ptr then
        error('LuaVM CData pusher: Invalid ctype')
    end

    C.lua_getfield(s, -10002, 'NX_CData')
    C.lua_pushlightuserdata(s, ptr)
    C.lua_pushstring(s, type)
    C.lua_pushboolean(s, dereference)

    if C.lua_pcall(s, 3, 1, 0) ~= 0 then
        error('LuaVM CData pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
    end
end

-- Retrieves the table at index i from the lua_State s
local function tableRetriever(s, i)
    -- If index is relative the end of the stack, make it absolute
    if i < 0 then i = C.lua_gettop(s) + i + 1 end

    local result = {}

    C.lua_pushnil(s)
    while C.lua_next(s, i) ~= 0 do
        local keyType = typeof(s, -2)
        local keyRetriever = retrievers[keyType]
        if keyRetriever then
            local key = keyRetriever(s, -2)
            local valType = typeof(s, -1)
            local valRetriever = retrievers[valType]

            if valRetriever then
                result[key] = valRetriever(s, -1)
            else
                error('LuaVM Table retriever: value type ' .. valType .. ' for key ' .. key ..
                    ' is not supported.')
            end
        else
            error('LuaVM Table retriever: key type ' .. keyType .. ' is not supported.')
        end

        C.lua_settop(s, -2)
    end

    return result
end

-- Retrieves the function at index i from the LuaVM s
local function funcRetriever(s, i)
    local top = C.lua_gettop(s)
    if i < 0 then i = top + i + 1 end

    C.lua_getfield(s, -10002, 'string')
    C.lua_getfield(s, -1, 'dump')
    C.lua_pushvalue(s, i)
    if C.lua_pcall(s, 1, 1, 0) == 0 then
        error('LuaVM Function pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
    end

    local lenB = ffi.new('size_t[1]')
    local strPtr = C.lua_tolstring(s, -1, lenB)
    local dump = ffi.string(strPtr, lenB[0])
    C.lua_settop(s, top)

    local func, err = loadstring(dump)
    if func then
        return func
    else
        error('LuaVM Function retriever: ' .. err)
    end
end

-- Wrappers for pushing values by type
pushers = {
    ['nil']      = function(s, v) C.lua_pushnil(s) end,
    ['cdata']    = cdataPusher,
    ['table']    = tablePusher,
    ['number']   = C.lua_pushnumber,
    ['string']   = C.lua_pushstring,
    ['boolean']  = C.lua_pushboolean,
    ['function'] = funcPusher,
    ['nxclass']  = nxClassPusher
}

-- Wrappers for retrieving values by type
retrievers = {
    ['nil']      = function(s, i) return nil end,
    ['cdata']    = C.nxLuaToCdata,
    ['userdata'] = C.lua_touserdata,
    ['table']    = tableRetriever,
    ['number']   = C.lua_tonumber,
    ['string']   = function(s, i) return ffi.string(C.lua_tolstring(s, i, nil)) end,
    ['boolean']  = function(s, i) return C.lua_toboolean(s, i) == 1 end,
    ['function'] = funcRetriever
}

function LuaVM:initialize()
    local handle = C.luaL_newstate()
    if handle == nil then
        Log.warning('Cannot create new Lua VM')
    else
        if C.nxLuaLoadNxLibs(handle) then
            self._cdata = ffi.gc(handle, C.lua_close)
        else
            Log.warning('Unable to load NxLibs into new Lua VM')
            C.lua_close(handle)
        end

        -- Empty the stack
        self:setTop(0)
    end
end

function LuaVM:release()
    if self._cdata == nil then return end

    C.lua_close(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

function LuaVM:isOpen()
    return self._cdata ~= nil
end

function LuaVM:top()
    if self._cdata == nil then return 0 end

    return C.lua_gettop(self._cdata) - 1
end

function LuaVM:setTop(index)
    if self._cdata ~= nil then
        -- There's a reseved slot in the stack for the debug function
        C.lua_settop(self._cdata, index + 1)
    end

    return self
end

-- Pushes values into the stack
function LuaVM:push(...)
    if self._cdata == nil then return 0 end

    local args = {...}
    local argc = table.maxn(args)

    -- Needed to revert to initial state in case of error
    local top = C.lua_gettop(self._cdata)

    for i = 1, argc do
        local val = args[i]
        local typename = type(val)

        local pushFunc = pushers[typename]
        if not pushFunc then
            error('Cannot push value to LuaVM: type ' .. typename ..
                'is not supported. Aborting')
        end

        pushFunc(self._cdata, val)
    end

    return argc
end

-- popped values are returned if returnValues is true
function LuaVM:pop(count, returnValues)
    if self._cdata == nil then return end

    if count == 0 then return end
    count = count or 1 -- if count == nil

    if returnValues then
        local retval = {}
        local retCount = 1

        for i = -count, -1 do
            local typename = typeof(self._cdata, i)
            local retriever = retrievers[typename]

            if retriever then
                retval[retCount] = retriever(self._cdata, i)
            else
                error('LuaVM:pop(): Type ' .. typename .. 'Unsupported. Skipping...')
            end

            retCount = retCount + 1
        end

        C.lua_settop(self._cdata, -count - 1)
        return unpack(retval, 1, count)
    else
        C.lua_settop(self._cdata, -count - 1)
    end
end

-- Calls func with the following parameters as arguments
--  func must not contain upvalues (eg.: values from external scope)
--  Returns the err if a problem occurs at calling
--  Returned values are pushed to the stack, and their count is returned
function LuaVM:pcall(func, ...)
    if self._cdata == nil then return 0 end
    local top = C.lua_gettop(self._cdata)

    local argc, err = self:push(func, ...)
    if not argc then return nil, err end

    local args = {func, ...}
    if C.lua_pcall(self._cdata, argc - 1, -1, 0) ~= 0 then
        return nil, self:pop(1, true)
    end

    return C.lua_gettop(self._cdata) - top
end

function LuaVM:call(func, ...)
    local argCount, err = self:pcall(func, ...)
    if not argCount then error(err) end

    return self:pop(argCount, true)
end

return LuaVM
