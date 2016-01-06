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

local Worker = {}
local registeredTypes = {}
local totalTasks, finishedTasks, failedTasks = 0, 0, 0
local loadingTasks, generalTasks = {}, {}

local function loadFunc(stagePtr, proc, obj, name, deps, params)
    if proc(obj, name, unpack(deps), params and unpack(params)) then
        stagePtr[0] = stagePtr + 1
    else
        stagePtr[0] = 0
    end
end

function Worker.registerType(type, factoryFunc)
    registeredTypes[type] = factoryFunc
end

function Worker.prepare()
    totalTasks, finishedTasks, failedTasks = 0, 0, 0
    for i in pairs(loadingTasks) do
        totalTasks = totalTasks + 1
    end
    for i in pairs(generalTasks) do
        totalTasks = totalTasks + 1
    end
end

function Worker.progress()
    return totalTasks, finishedTasks, failedTasks
end

function Worker.hasTasks()
    for i in pairs(loadingTasks) do
        return true
    end
    for i in pairs(generalTasks) do
        return true
    end
    return false
end

function Worker.addLoadingTask(screen, id)
    -- Check if task already exists
    for i, task in pairs(loadingTasks) do
        if task.id == id then
            return task.obj, task.reusable
        end
    end

    -- Does not exists, add it the task list
    local type = id:match('[^:]+')

    local factoryFunc = registeredTypes[type]
    if not factoryFunc then
        error('Attempting to load object of unregistered type "' .. type .. '"')
    end

    local name = id:sub(#type+2)
    local task = factoryFunc(name)
    task.id = id
    task.stagePtr = ffi.new('uint32_t[1]', 1)
    task.lastStage = 0
    task.screen = screen
    task.depsAdded = false
    task.deps = task.deps or {}
    task.name = task.name or name
    task.reusable = task.reusable == nil or task.reusable

    loadingTasks[table.maxn(loadingTasks)+1] = task
    return task.obj, task.reusable
end

function Worker.iteration()
    -- Reverse iterate because the latter are less likely to be waiting
    -- for dependencies to load
    for i = table.maxn(loadingTasks), 1, -1 do
        local task = loadingTasks[i]
        if task then
            -- Cache func
            local cache = task.screen and task.screen.cache or require('game.cache').get

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
                local item = cache(task.screen, dep, true)

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

                if stage > #task.funcs then
                    -- Task is done, remove it froom ongoing tasks
                    task.obj.__wk_status = 'ready'
                    loadingTasks[i] = nil
                    finishedTasks = finishedTasks + 1

                    -- Remove temporary depndencies
                    for dep, temporary in pairs(task.deps) do
                        if temporary then
                            if task.screen then
                                task.screen:uncache(dep)
                            else
                                Cache.release(dep)
                            end
                        end
                    end
                else
                    local func = task.funcs[stage]
                    local deps = {}
                    if func.deps and #func.deps > 0 then
                        for i, dep in ipairs(func.deps) do
                            deps[#deps+1] = cache(task.screen, dep, true)
                        end
                    end

                    -- TODO: Re-implement gpu-multithreading (on non-android devices)
                    if func.threaded and func.threaded ~= 'gpu' then
                        Thread:new(
                            loadFunc, task.stagePtr, func.proc, task.obj, task.name,
                            deps, func.params
                        ):detach()
                    else
                        loadFunc(
                            task.stagePtr, func.proc, task.obj, task.name,
                            deps, func.params
                        )
                    end
                end
            end
        end
    end

    return Worker.progress()
end

Worker.registerType('image', function(filename)
    return {
        obj = require('graphics.image'):new(),
        funcs = {
            {
                proc = function(image, filename)
                    image:load(filename)
                end,
                threaded = true
            }
        }
    }
end)

Worker.registerType('vectorfont', function(filename)
    return {
        obj = require('graphics.vectorfont'):new(),
        funcs = {
            {
                proc = function(font, filename)
                    font:open(filename)
                end,
                threaded = false
            }
        }
    }
end)

return Worker