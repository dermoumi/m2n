local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct lua_State lua_State;

    bool nxLuaLoadNxLibs(lua_State*);
    void* nxLuaToCdata(lua_State*, int);

    lua_State* luaL_newstate();
    void luaL_openlibs(lua_State*);
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

--------------------------------------------------------------------------------

local pushers = {}
local retrievers = {}

-- Returns the typename of the value at the given index of lua_State s
local function typeof(s, i)
    return ffi.string(C.lua_typename(s, C.lua_type(s, i)))
end

-- Pushes the table t into the lua_State s
local function tablePusher(s, t)
    C.lua_createtable(s, 0, 0)
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
        C.lua_settop(s, -2)
    end

    -- Remove the second return value in case there were no errors
    C.lua_settop(s, -2)
end

-- Pushes the nxLib object into the lua_State s
local function nxObjPusher(s, o)
    local className = tostring(o.class.name)
    local cdata = o:_cdata()

    C.lua_getfield(s, -10002, 'NX_LibObject')
    C.lua_pushlightuserdata(s, cdata)
    C.lua_pushstring(s, className)

    if C.lua_pcall(s, 2, 1, 0) ~= 0 then
        error('LuaVM nxLib Object pusher: ' .. ffi.string(C.lua_tolstring(s, -1, nil)))
        C.lua_settop(s, -2)
    end
    print('classname: ' .. className)
end

-- Retrieves the table at index i from the lua_State s
local function tableRetriever(s, i)
    -- If index is relative the end of the stack, make it absolute
    if i < 0 then i = C.lua_gettop(s) + i + 1 end

    local result = {}

    C.lua_pushnil(s)
    while C.lua_next(s, i) do
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
        C.lua_settop(s, -2)
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
    ['cdata']    = C.lua_pushlightuserdata,
    ['table']    = tablePusher,
    ['number']   = C.lua_pushnumber,
    ['string']   = C.lua_pushstring,
    ['boolean']  = C.lua_pushboolean,
    ['function'] = funcPusher,
    ['nxobj']    = nxObjPusher
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

--------------------------------------------------------------------------------

local LuaVM = class 'nx.luavm'

function LuaVM.static._fromCData(data)
    local vm = LuaVM:allocate()
    vm._handle = ffi.cast('lua_State*', data)
    return vm
end

function LuaVM:initialize(init)
    local handle = C.luaL_newstate()
    if handle == nil then
        return nil, 'Cannot create new Lua VM'
    end

    C.luaL_openlibs(handle)

    if C.nxLuaLoadNxLibs(handle) then
        self._handle = ffi.gc(handle, C.lua_close)
    else
        C.lua_close(handle)
    end

    self:setTop(0)
end

function LuaVM:_cdata()
    return self._handle
end

function LuaVM:isOpen()
    return self._handle ~= nil
end

function LuaVM:getTop()
    return C.lua_gettop(self._handle)
end

function LuaVM:setTop(index)
    C.lua_settop(self._handle, index)
end

-- Pushes values into the stack
function LuaVM:push(...)
    local args = {...}
    local argc = table.maxn(args)

    local top = C.lua_gettop(self._handle)

    for i = 1, argc do
        local val = args[i]
        local typename = type(val)

        -- If has a :_cdata() function, then it's a wrapper class
        -- and we should push the handle instead
        if typename == 'table' and val._cdata and type(val._cdata) == 'function' then
            if val == self then
                C.lua_settop(self._handle, top)
                return nil, 'Cannot push a LuaVM into itself'
            else
                typename = 'nxobj'
            end
        end

        local pushFunc = pushers[typename]
        if not pushFunc then
            C.lua_settop(self._handle, top)
            return nil, 'Cannot push value to LuaVM: type ' .. typename .. 
                'is not supported. Aborting'
        end

        pushFunc(self._handle, val)
    end

    return argc
end

-- Pops values from th stack, if returnValues is true, popped values are returned
function LuaVM:pop(count, returnValues)
    if count == 0 then return end
    count = count or 1 -- if count == nil

    if returnValues then
        local retval = {}
        local retCount = 1

        for i = -count, -1 do
            local typename = typeof(self._handle, i)
            local retriever = retrievers[typename]

            if retriever then
                retval[retCount] = retriever(self._handle, i)
            else
                error('LuaVM:pop(): Type ' .. typename .. 'Unsupported. Skipping...')
                retval[retCount] = nil
            end

            retCount = retCount + 1
        end

        C.lua_settop(self._handle, -count - 1)
        return unpack(retval, 1, count)
    else
        C.lua_settop(self._handle, -count - 1)
    end
end

-- Calls func with the following parameters as arguments
--  func must not contain upvalues (eg.: values from external scope)
--  Returns the err if a problem occurs at calling
--  Returned values are pushed to the stack, and their count is returned
function LuaVM:call(func, ...)
    local top = C.lua_gettop(self._handle)

    local argc, err = self:push(func, ...)
    if not argc then return nil, err end

    local args = {func, ...}
    local retCount = 0

    local ok = C.lua_pcall(self._handle, argc - 1, -1, 0)
    if ok ~= 0 then
        local err = self:pop(1, true)
        return nil, err
    end

    return C.lua_gettop(self._handle) - top
end

return LuaVM
