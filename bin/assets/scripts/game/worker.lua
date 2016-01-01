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

local ffi    = require 'ffi'
local class  = require 'class'
local Thread = require 'system.thread'

local Worker = class 'Worker'

local loaderFunc = {}

local function genericTaskFunc(loadedCount, func, ...)
    loadedCount = require('ffi').cast('uint32_t*', loadedCount)
    if not func(...) then
        loadedCount[1] = loadedCount[1] + 1
    else
        loadedCount[0] = loadedCount[0] + 1
    end
end

local function genericLoadingFunc(loaderFunc, obj, filename, loadedCount)
    loadedCount = require('ffi').cast('uint32_t*', loadedCount)
    if not loaderFunc(obj, filename) then
        loadedCount[1] = loadedCount[1] + 1
    else
        loadedCount[0] = loadedCount[0] + 1
    end
end

local function callLoadFunc(obj, id)
    obj:load(id)
    return true
end

local function returnFalseFunc()
    return false
end

function Worker.static.registerFunc(objType, func)
    loaderFunc[objType] = func
    return Worker
end

function Worker:initialize()
    self._tasks = {}
    self._taskCount = 0
    self._loadedCount = ffi.new('uint32_t[2]')
end

function Worker:addFile(objType, id)
    local Cache = require('game.cache')

    -- Check if already loaded in the cache
    local obj = Cache.get(id)
    if obj then return end

    -- Not loaded. Check if loader function is valid
    obj = require(objType):new()
    local func = loaderFunc[objType]
    if not func then
        -- No registered loading func, if obj has a :load() method, use it. Otherwise, abort.
        func = (type(obj.load) == 'function') and callLoadFunc or returnFalseFunc
        loaderFunc[objType] = func
    end

    -- Valid, push a new task to load it
    self:checkCount()
    self._taskCount = self._taskCount + 1

    self._tasks[#self._tasks+1] = {genericLoadingFunc, func, obj, id, self._loadedCount}

    Cache.add(id, obj)
end

function Worker:addTask(taskFunc, ...)
    self:checkCount()
    self._taskCount = self._taskCount + 1
    
    self._tasks[#self._tasks+1] = {genericTaskFunc, self._loadedCount, taskFunc, ...}
end

function Worker:start()
    -- Make a thread for each task, and detach it...
    for i, task in ipairs(self._tasks) do
        Thread:new(unpack(task, 1, table.maxn(task))):detach()
    end

    -- Clear tasks
    self._tasks = {}
    self._shouldReset = true
end

function Worker:progress()
    return tonumber(self._loadedCount[0]), tonumber(self._loadedCount[1]), self._taskCount
end

function Worker:taskCount()
    return self._taskCount
end

function Worker:checkCount()
    if self._shouldReset then
        self._loadedCount[0] = 0
        self._loadedCount[1] = 0
        self._taskCount = 0
    end
end

return Worker