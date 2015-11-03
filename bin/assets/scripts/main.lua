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
local Scene      = require 'scene'

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
-- Renderer setup
------------------------------------------------------------

-- Create window
local ok, err = Window.create("m2n", 1280, 720, {
})

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

Renderer.setupViewport(0, 0, Window.size())

------------------------------------------------------------
-- Startup scene
------------------------------------------------------------
Scene.goTo('scene.title')

------------------------------------------------------------
-- Handling FPS
------------------------------------------------------------
local framerateLimit
if Nx.getPlatform() == 'android' or Nx.getPlatform() == 'ios' then
    framerateLimit = 1/30
else
    framerateLimit = 1/60
end

local totalElapsedTime = 0
local frameCount       = 0
local lastTime         = Nx.getSystemTime()

------------------------------------------------------------
-- Main loop
------------------------------------------------------------
while true do
    local currentTime = Nx.getSystemTime()
    local elapsedTime = currentTime - lastTime
    lastTime = currentTime

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
    
    scene:_update(elapsedTime)

    Renderer.begin()
    scene:_render()
    Renderer.finish()

    Window.display()

    -- Calculating FPS every whole second
    totalElapsedTime = totalElapsedTime + elapsedTime
    frameCount       = frameCount + 1
    if totalElapsedTime > 1 then
        local currentFPS = math.floor(1 / (totalElapsedTime / frameCount) + .5)
        Window.setTitle('m2n [' .. currentFPS .. ']')

        -- Print the FPS when on android
        if Nx.getPlatform() == 'android' then
            print('Current FPS: ' .. currentFPS)
        end
        
        totalElapsedTime = totalElapsedTime % 1
        frameCount = 0
    end

    -- Waiting out left time of the frame
    local sleepTime = currentTime - Nx.getSystemTime() + framerateLimit;
    if sleepTime > 0 then
        Nx.sleep(sleepTime)
    end
end

------------------------------------------------------------
-- The end?
------------------------------------------------------------
return 0