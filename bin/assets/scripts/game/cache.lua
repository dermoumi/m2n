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
local Log    = require 'util.log'
local Thread = require 'system.thread'
local class  = require 'class'

local Cache = {}
local items = {}

local registeredTypes = {}
local totalTasks, finishedTasks, failedTasks = 0, 0, 0
local loadingTasks = {}

local Task = class '_cacheclass'

function Task:initialize(id, name, objClass, screen)
    self.id = id
    self.obj = objClass:new()
    self.stagePtr = ffi.new('uint32_t[1]', 1)
    self.lastStage = 0
    self.screen = screen
    self.depsAdded = false
    self.name = name
    self.reusable = true
    self.deps = {}
    self.tasks = {}
end

function Task:addTask(threaded, func, deps)
    if type(threaded) == 'function' then
        threaded, func, deps = false, threaded, func
    end

    self.tasks[#self.tasks+1] = {
        func = func,
        threaded = threaded,
        deps = deps
    }

    return self
end

function Task:setReusable(reusable)
    self.reusable = reusable
    return self
end

function Task:addDependency(id, temporary)
    self.deps[id] = not not temporary
    return self
end

local function loadFunc(stagePtr, proc, obj, name, deps, params)
    -- If there are no dependencies, move params to theirs slots instead
    if #deps == 0 then
        deps, params = params, {}
    end

    if proc(obj, name, unpack(deps)) == false then
        stagePtr[0] = 0
    else
        stagePtr[0] = stagePtr[0] + 1
    end
end

local function addLoadingTask(screen, id)
    -- Check if task already exists
    for i, task in pairs(loadingTasks) do
        if task.id == id then
            return task.obj, task.reusable
        end
    end

    -- Does not exists, add it the task list
    local type = id:match('[^:]+')

    local objClass = registeredTypes[type]
    if not objClass then
        error('Attempting to load object of unregistered type "' .. type .. '"')
    end

    local name = id:sub(#type+2)
    local task = Task:new(id, name, objClass, screen)
    objClass.factory(task, name, type)

    loadingTasks[table.maxn(loadingTasks)+1] = task
    return task.obj, task.reusable
end

function Cache.registerType(objType, objClass)
    if type(objClass) == 'string' then
        objClass = require(objClass)
    end
    registeredTypes[objType] = objClass
    return Cache
end

function Cache.prepare()
    totalTasks, finishedTasks, failedTasks = 0, 0, 0
    for i in pairs(loadingTasks) do
        totalTasks = totalTasks + 1
    end
end

function Cache.progress()
    return totalTasks, finishedTasks, failedTasks
end

function Cache.hasTasks()
    for i in pairs(loadingTasks) do
        return true
    end
    return false
end

function Cache.iteration()
    -- Reverse iterate because the latter are less likely to be waiting
    -- for dependencies to load
    for i = table.maxn(loadingTasks), 1, -1 do
        local task = loadingTasks[i]
        if task then
            -- Cache func
            local cache = task.screen and task.screen.cache or Cache.get

            -- Add dependencies
            if not task.depsAdded then
                task.depsAdded = true

                for dep in pairs(task.deps) do
                    cache(task.screen, dep)
                end
            end

            -- Check how many dependencies haave successfully loaded
            local ready = true
            for dependency in pairs(task.deps) do
                local item = cache(task.screen, dependency, true)

                local status = not item and 'failed' or item.__wk_status
                if status == 'failed' then
                    task.stagePtr[0] = 0
                    break
                elseif status ~= 'ready' then
                    ready = false
                    break
                end
            end

            if task.stagePtr[0] == 0 then
                -- Failed
                task.obj.__wk_status = 'failed'
                loadingTasks[i] = nil
                failedTasks = failedTasks + 1
                finishedTasks = finishedTasks + 1
            elseif task.stagePtr[0] ~= task.lastStage and ready then
                -- If the stage number has changed between last time and now
                local stage = task.stagePtr[0]
                task.lastStage = stage

                if stage > #task.tasks then
                    -- Task is done, remove it froom ongoing tasks
                    task.obj.__wk_status = 'ready'
                    loadingTasks[i] = nil
                    finishedTasks = finishedTasks + 1
                    Log.info('Loaded: ' .. task.id)

                    -- Remove temporary depndencies
                    for dep, temporary in pairs(task.deps) do
                        if temporary then
                            finishedTasks = finishedTasks - 1
                            if task.screen then
                                task.screen:uncache(dep)
                            else
                                Cache.release(dep)
                            end
                        end
                    end
                else
                    local subTask = task.tasks[stage]
                    local deps = {}
                    if subTask.deps and #subTask.deps > 0 then
                        for i, dep in ipairs(subTask.deps) do
                            deps[#deps+1] = cache(task.screen, dep, true)
                        end
                    end
                    subTask.params = subTask.params or {}

                    -- TODO: Re-implement gpu-multithreading (on non-android devices)
                    if subTask.threaded and subTask.threaded ~= 'gpu' then
                        Thread:new(
                            loadFunc, task.stagePtr, subTask.func, task.obj, task.name,
                            deps, subTask.params
                        ):detach()
                    else
                        loadFunc(
                            task.stagePtr, subTask.func, task.obj, task.name,
                            deps, subTask.params
                        )
                    end
                end
            end
        end
    end

    return Cache
end

function Cache.wait()
    while Cache.hasTasks() do
        Cache.iteration()
    end
end

function Cache.get(screen, id, peek)
    if type(screen) == 'string' then
        screen, id, peek = nil, screen, id
    end

    local item = items[id]

    -- If item does not exist, try to load it using loadFunc
    if not item then
        items[id] = item
        local obj, reusable = addLoadingTask(screen, id)
        if reusable then
            Log.info('Adding to global cache: ' .. id)
            item = {
                count = 0,
                obj = obj
            }

            items[id] = item
        else
            return obj, false
        end
    end

    -- If requested, increment the load count of the item
    if not peek then
        item.count = item.count + 1
    end

    -- Return the item's object
    return item.obj, true
end

function Cache.release(id)
    local item = items[id]
    if not item then return end

    -- Decrement load count
    item.count = item.count - 1

    -- If load count reaches zero, remove item from list
    if item.count <= 0 then
        Log.info('Removing from global cache: ' .. id)

        items[id] = nil

        -- If object can be released, do that
        if item.obj.release then item.obj:release() end
    end
end

return Cache
