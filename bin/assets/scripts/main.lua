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

local Nx       = require 'nx'
local Log      = require 'nx.log'
local Window   = require 'nx.window'
local Events   = require 'nx.events'
local Renderer = require 'nx.renderer'
local Audio    = require 'nx.audio'
local Scene    = require 'scene'

------------------------------------------------------------
-- Helpers
------------------------------------------------------------

-- Mapping events to their respecting nx.Scene functions
local eventMapping = {
    focus             = 'onFocus',
    visible           = 'onVisible',
    resized           = 'onResize',
    textinput         = 'onTextInput',
    textedit          = 'onTextEdit',
    keydown           = 'onKeyDown',
    keyup             = 'onKeyUp',
    mousefocus        = 'onMouseFocus',
    mousemotion       = 'onMouseMotion',
    mousedown         = 'onMouseDown',
    mouseup           = 'onMouseUp',
    wheelscroll       = 'onWheelScroll',
    joyaxismotion     = 'onJoyAxisMotion',
    joyballmotion     = 'onJoyBallMotion',
    joyhatmotion      = 'onJoyHatMotion',
    joybuttondown     = 'onJoyButtonDown',
    joybuttonup       = 'onJoyButtonUp',
    joyconnect        = 'onJoyConnect',
    gamepadmotion     = 'onGamepadMotion',
    gamepadbuttondown = 'onGamepadButtonDown',
    gamepadbuttonup   = 'onGamepadButtonUp',
    gamepadconnect    = 'onGamepadConnect',
    gamepadremap      = 'onGamepadRemap',
    touchdown         = 'onTouchDown',
    touchup           = 'onTouchUp',
    touchmotion       = 'onTouchMotion',
    filedrop          = 'onFileDrop'
}

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
-- Renderer setup
------------------------------------------------------------

-- Create window
local ok, err = Window.create("m2n", 1280, 720, graphicsFlags)
if not ok then
    Log.error('Cannot initialize window: ' + err)
    return 1
end

-- Initialize renderer
ok = Renderer.init()
if not ok then
    Log.error('Cannot initialize renderer.')
    return 1
end

Window.setIcon('assets/icon.png')

------------------------------------------------------------
-- Handling FPS
------------------------------------------------------------
Window.setFramerateLimit(
    (Nx.getPlatform() == 'android' or Nx.getPlatform() == 'ios') and 1/30 or 1/60
)

------------------------------------------------------------
-- Startup scene
------------------------------------------------------------
local worker = require('game.worker'):new()

if Nx.getPlatform() == 'windows' then
    -- On windows initializing SDL Audio takes some time, load it in a worker
    worker:addTask(function()
        if not require('nx.audio').init() then
            require('nx.log').error('Could not initialize sound system')
            return false
        end
        
        return true
    end)
else
    require('nx.audio').init()
end

-- Makes the initial loading slower for testing purpose
-- TODO: Remove this when finished
worker:addTask(function()
    require('nx').sleep(2)
    return true
end)

Scene.goTo('scene.load', worker, function()
    Scene.goTo('scene.title')
end)

------------------------------------------------------------
-- Main loop
------------------------------------------------------------
while true do
    local scene = Scene.currentScene()

    -- Process events
    for e, a, b, c, d in Events.poll() do
        if e == 'quit' and scene:_onEvent('onQuit') then
            Window.close()
            break
        else
            local event = eventMapping[e] or 'onEvent'
            scene:_onEvent(event, a, b, c, d)
        end
    end

    -- Check if the window is still open
    if not Window.isOpen() then break end
    
    scene:_update(Window.frameTime())

    Renderer.begin()
    scene:_render()
    Renderer.finish()

    Window.display()

    -- Clean the scene stack
    Scene.clean()
end

------------------------------------------------------------
-- Release stuff
------------------------------------------------------------
Audio.release()

------------------------------------------------------------
-- The end?
------------------------------------------------------------
return 0