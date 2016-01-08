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
local Config = require 'config'
local Thread = require 'system.thread'
local LuaVM  = require 'system.luavm'
local class  = require 'class'

local Cache = {}
local items = {}

local registeredTypes = {}
local totalTasks, finishedTasks, failedTasks = 0, 0, 0
local loadingTasks, temporaryDeps = {}, {}

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
    self.newDeps = {}
    self.tasks = {}
    self.params = {}
    self.vm = LuaVM:new()
end

function Task:addTask(threaded, func, deps)
    if type(threaded) == 'function' then
        threaded, func, deps = false, threaded, func
    end

    self.tasks[#self.tasks+1] = {
        func = func,
        threaded = threaded
    }

    return self
end

function Task:setReusable(reusable)
    self.reusable = reusable
    return self
end

function Task:addParam(...)
    local params = {...}
    local count, paramCount = table.maxn(params), table.maxn(self.params)

    for i = 1, count do
        local param = params[i]
        if type(param) == 'string' and param:match('.:.') then
            local temporary = false
            if param:match('^#.') then
                temporary = true
                param = param:sub(2)
            end
            self:addDependency(param, temporary)
        end
        self.params[paramCount+i] = param
    end

    return self
end

function Task:addDependency(id, temporary)
    if id:match('^#.') then
        temporary = true
        id = id:sub(2)
    end

    if self.deps[id] ~= false then
        self.deps[id] = not not temporary
    end

    return self
end

local function loadFunc(stagePtr, vm, gpu, proc, obj, name, params)
    if gpu then require('window').ensureContext() end

    local retVals = {proc(obj, name, unpack(params))}
    if retVals[1] == false then
        stagePtr[0] = 0
    else
        vm:push(unpack(retVals))
        stagePtr[0] = stagePtr[0] + 1
    end

    -- Synchronize loaded data accross all contexts
    if gpu then require('graphics').sync() end
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
    -- print(totalTasks, finishedTasks, failedTasks)
    return totalTasks, finishedTasks, failedTasks
end

function Cache.hasTasks()
    for i in pairs(loadingTasks) do
        return true
    end
    return false
end

local function checkTemporary(dep, screen, temporary)
    if not screen then screen = false end

    if temporary then
        if not temporaryDeps[screen] then
            temporaryDeps[screen] = {}
        end
        temporaryDeps[screen][dep] = true
    elseif temporaryDeps[screen] then
        temporary[screen][dep] = false
    end
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

                for dep, temporary in pairs(task.deps) do
                    cache(task.screen, dep)

                    checkTemporary(dep, task.screen, temporary)
                end
            end

            -- Add dependencies that are added during a subTask
            for dep, temporary in pairs(task.newDeps) do
                cache(task.screen, dep)
                if not task.deps[dep] then
                    task.deps[dep] = temporary
                    task.newDeps[dep] = nil

                    checkTemporary(dep, task.screen, temporary)
                end
            end

            -- Check how many dependencies have successfully loaded
            local ready = true
            for dependency in pairs(task.deps) do
                local item = cache(task.screen, dependency, true)

                if not item or item.__wk_status == 'failed' then
                    task.stagePtr[0] = 0
                    break
                elseif item.__wk_status ~= 'ready' then
                    ready = false
                    break
                end
            end

            if task.stagePtr[0] == 0 then
                -- Failed
                task.obj.__wk_status = 'failed'
                loadingTasks[i] = nil
                failedTasks = failedTasks + 1
                Log.error('Failed to load file: ' .. task.id)
            elseif task.stagePtr[0] ~= task.lastStage and ready then
                -- If the stage number has changed between last time and now
                local stage = task.stagePtr[0]
                task.lastStage = stage

                if stage > #task.tasks then
                    -- Task is done, remove it from ongoing tasks
                    task.obj.__wk_status = 'ready'
                    loadingTasks[i] = nil
                    Log.info('Loaded: ' .. task.id)
                else
                    local subTask = task.tasks[stage]
                    local params = {}
                    local depsChanged = false

                    local entries = (stage == 1) and task.params or subTask.entries
                    if entries then
                        for i, entry in ipairs(entries) do
                            if type(entry) == 'string' and entry:match('.:.') then
                                params[#params+1] = cache(task.screen, entry, true)
                            elseif not depsChanged then
                                params[#params+1] = entry
                            end
                        end
                    else
                        -- Pop IDs from the vm and repopulate it with instances
                        subTask.entries = {task.vm:pop(task.vm:top(), true)}
                        for i, entry in ipairs(subTask.entries) do
                            if type(entry) == 'string' and entry:match('.:.') then
                                depsChanged = true
                                local temporary = false
                                if entry:match('^#.') then
                                    temporary = true
                                    entry = entry:sub(2)
                                end
                                if task.deps[entry] ~= false then
                                    task.newDeps[entry] = temporary
                                end
                            elseif not depsChanged then
                                params[#params+1] = entry
                            end
                        end
                    end

                    if not depsChanged then
                        -- TODO: Re-implement gpu-multithreading (on non-android devices)
                        local gpu = subTask.threaded == 'gpu' and not Config.noGpuMultithreading
                        if subTask.threaded == true or gpu then
                            Thread:new(
                                loadFunc, task.stagePtr, task.vm, gpu,
                                subTask.func, task.obj, task.name, params
                            ):detach()
                        else
                            loadFunc(
                                task.stagePtr, task.vm, false,
                                subTask.func, task.obj, task.name, params
                            )
                        end
                    else
                        task.lastStage = 0
                    end
                end
            end
        end
    end

    local taskCount = table.maxn(loadingTasks)
    if taskCount == 0 then
        -- Remove temporary depndencies
        for screen, deps in pairs(temporaryDeps) do
            for dep in pairs(deps) do
                if screen then
                    screen:uncache(dep)
                else
                    Cache.release(dep)
                end
            end
        end
        temporaryDeps = {}
    end

    if totalTasks > taskCount then
        finishedTasks = totalTasks - taskCount
    else
        totalTasks = taskCount
    end

    return Cache
end

function Cache.wait()
    while Cache.hasTasks() do
        Cache.iteration()
    end

    return Cache
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
