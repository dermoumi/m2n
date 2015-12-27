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

local System   = require 'nx.system'
local Log      = require 'nx.util.log'
local Events   = require 'nx.window.events'
local Graphics = require 'nx.graphics'
local Window   = require 'nx.window'
local Audio    = require 'nx.audio'
local Scene    = require 'scene'

------------------------------------------------------------
-- Load settings
------------------------------------------------------------
local graphicsFlags

local settings, err = loadfile('userdata/settings.lua')
if not settings then
    Log.warning('Could not load user settings: ' .. err)
else
    settings = settings()
    graphicsFlags = settings.graphics
end

------------------------------------------------------------
-- Setup
------------------------------------------------------------
-- Create window
Window.create("m2n", 1280, 720, graphicsFlags or {vsync = false})

-- Initialize renderer
Graphics.init()

-- Set window icon
Window.setIcon('assets/icon.png')

-- Initialize audio module
if not Audio.init() then
    Log.error('Could not initialize sound system')
end

------------------------------------------------------------
-- Handling FPS
------------------------------------------------------------
local totalTime = 0
local fixedFrameTime = 1/30
Window.setFramerateLimit(
    -- System.platform('android', 'ios') and 1/30 or 1/60
)

------------------------------------------------------------
-- Scene checker
------------------------------------------------------------
local function checkScene(scene)
    return scene ~= Scene.currentScene()
end

------------------------------------------------------------
-- Startup scene
------------------------------------------------------------
Scene.goTo('scene.title', true)

------------------------------------------------------------
-- Main loop
------------------------------------------------------------
while Window.isOpen() do
    local scene = Scene.currentScene()

    -- Process events
    for e, a, b, c, d in Events.poll() do
        if e == 'quit' and scene:__onEvent('quit') then
            Window.close()
            break
        else
            scene:__onEvent(e, a, b, c, d)
            if checkScene(scene) then goto continue end
        end
    end

    -- Check that the window is still open
    if not Window.isOpen() then break end

    scene:__update(Window.frameTime())
    if checkScene(scene) then goto continue end

    totalTime = totalTime + Window.frameTime()
    for i = 1, totalTime / fixedFrameTime do
        scene:__fixedUpdate(fixedFrameTime)
        if checkScene(scene) then goto continue end
    end
    totalTime = totalTime % fixedFrameTime

    Graphics.begin()
    scene:__render()
    Graphics.finish()

    Window.display()

    ::continue::
end

------------------------------------------------------------
-- Release stuff
------------------------------------------------------------
Audio.release()

------------------------------------------------------------
-- The end?
------------------------------------------------------------
return 0
