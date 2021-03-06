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

local Log = require 'util.log'

local noFpsLimit

-- Handle application arguments
for i, v in ipairs(arg) do
    if v == '--version' then
        Log.info('Monsters of Second Night v0.0')
        return 0
    elseif v == '--nolimit' then
        noFpsLimit = true
    end
end

local System   = require 'system'
local LuaVM    = require 'system.luavm'
local Events   = require 'window.events'
local Graphics = require 'graphics'
local Window   = require 'window'
local Audio    = require 'audio'
local Screen   = require 'screen'
local Config   = require 'config'

-- Load settings (in VM sandbox)
local vm = LuaVM:new()
local argsCount = vm:pcall(function()
        local settings = loadfile('userdata/settings.lua')
        return settings and settings() or {}
    end)
local settings, err = vm:pop(argsCount, true)

-- Create window
Window.create("m2n", 1280, 720, {vsync = true})

-- Initialize renderer
Graphics.init()
Config.noGpuMultithreading = not Graphics.getCapabilities('multithreadingSupported')

-- Set window icon
Window.setIcon('assets/icon.png')

-- Initialize audio module
if not Audio.init() then
    Log.error('Could not initialize sound system')
end

-- Handling FPS
local totalTime, fixedFrameTime = 0, 1/30
if not noFpsLimit then
    Window.setFramerateLimit(System.platform('android', 'ios') and 1/30 or 1/60)
end

-- Register some types
require('game.cache')
    .registerType('image', 'graphics.image')
    .registerType('vectorfont', 'graphics.vectorfont')
    .registerType('fontstack', 'graphics.fontstack')
    .registerType('tex2d', 'graphics.texture2d')
    .registerType('music', 'audio.source')
    .registerType('sound', 'audio.source')
    .registerType('geom', 'graphics.geometry')
    .registerType('shader', 'graphics.shader')
    .registerType('material', 'graphics.material')
    .registerType('model', 'graphics.modeldesc')
    .registerType('scene', 'graphics.scenedesc')
    .registerType('state', 'game.state')

-- Register default input
require('game.input').register('game.input.keyboard', {
    up = 'up',
    left = 'left',
    right = 'right',
    down = 'down',
    w = 'up',
    a = 'left',
    d = 'right',
    s = 'down',
    escape = 'pause',
    space = 'accept',
    e = 'accept',
    tab = 'back',
    q = 'auxilary',
    shift = 'shift',
    ['return'] = 'accept'
})

-- Startup screen
Screen.goTo('screen.title', true)

-- Main loop
while Window.isOpen() do
    local screen = Screen.currentScreen()

    -- Process events
    for e, a, b, c, d in Events.poll() do
        if e == 'quit' and screen:__onEvent('quit') then
            Window.close()
            break
        else
            screen:__onEvent(e, a, b, c, d)
            if screen ~= Screen.currentScreen() then goto continue end
        end
    end

    -- Check that the window is still open
    if not Window.isOpen() then break end

    screen:__update(Window.frameTime())
    if screen ~= Screen.currentScreen() then goto continue end

    totalTime = totalTime + Window.frameTime()
    for i = 1, totalTime / fixedFrameTime do
        screen:__fixedUpdate(fixedFrameTime)
        if screen ~= Screen.currentScreen() then goto continue end
    end
    totalTime = totalTime % fixedFrameTime

    Graphics.begin()
    screen:__render()
    Graphics.finish()

    Window.display()

    ::continue::
    if screen ~= Screen.currentScreen() then
        Window.resetFrameTime()
    end
end

Audio.release()
