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
local Scene = require 'scene'
local SceneBoot = Scene:subclass('scene.boot')

------------------------------------------------------------
function SceneBoot:needsPreload()
    return true
end

------------------------------------------------------------
function SceneBoot:preload(worker)
    -- Initializing Audio is slow so we're doing it in a worker
    worker:addTask(function()
        if not require('nx.audio').init() then
            require('nx.log').error('Could not initialize sound system')
            return false
        end

        return true
    end)

    -- Makes the initial loading slower for testing purpose
    -- TODO: Remove this when finished
    worker:addTask(function()
       require('nx').sleep(1.4)
       return true
    end)
end

------------------------------------------------------------
function SceneBoot:load()
    local caps = require('nx.renderer').getCapabilities()

    local Log = require('nx.log')
    Log.info('================================')
    Log.info('GPU Capabilities:')
    Log.info('--------------------------------')
    for i, v in pairs(caps) do
        Log.info(i .. ': ' .. tostring(v))
    end

    Scene.goTo('scene.title')
end

------------------------------------------------------------
return SceneBoot
