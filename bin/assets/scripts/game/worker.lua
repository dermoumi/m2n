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
-- Allows to do tasks and loads files in threads
------------------------------------------------------------
local class = require 'nx.class'
local Worker = class 'Worker'

local Thread = require 'nx.thread'
local ffi = require 'ffi'

------------------------------------------------------------
local loaderFunc = {}

------------------------------------------------------------
function Worker.static.registerFunc(objType, func)
    loaderFunc[objType] = func
end

------------------------------------------------------------
function Worker:initialize()
    self._totalCount = 0
    self._loadedCount = ffi.new('uint32_t[2]')
end

------------------------------------------------------------
function Worker:addFile(objType, id)
    local Cache = require('game.cache')

    local obj = Cache.get(id)
    if obj then return end

    local loaderFunc = loaderFunc[objType] or function() return false end
    obj = require(objType):new()

    Thread:new(function(loaderFunc, obj, filename, loadedCount)
        loadedCount = require('ffi').cast('uint32_t*', loadedCount)
        if not loaderFunc(obj, filename) then
            loadedCount[1] = loadedCount[1] + 1
        else
            loadedCount[0] = loadedCount[0] + 1
        end
    end, loaderFunc, obj, id, self._loadedCount):detach()

    self._totalCount = self._totalCount + 1
    Cache.add(id, obj)
end

------------------------------------------------------------
function Worker:addTask(taskFunc, ...)
    Thread:new(function(loadedCount, func, ...)
        loadedCount = require('ffi').cast('uint32_t*', loadedCount)
        if not func(...) then
            loadedCount[1] = loadedCount[1] + 1
        else
            loadedCount[0] = loadedCount[0] + 1
        end
    end, self._loadedCount, taskFunc, ...):detach()

    self._totalCount = self._totalCount + 1
end

------------------------------------------------------------
function Worker:progress()
    return tonumber(self._loadedCount[0]), tonumber(self._loadedCount[1]), self._totalCount
end

------------------------------------------------------------
return Worker
