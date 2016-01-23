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

local Log       = require 'util.log'
local Graphics  = require 'graphics'
local Window    = require 'window'
local Screen    = require 'screen'
local GameFont  = require 'game.font'
local GameState = require 'game.state'

local ScreenTitle = Screen:subclass('screen.title')

function ScreenTitle:initialize(firstRun)
    if firstRun then
        local caps = Graphics.getCapabilities()

        Log.info('GPU Capabilities:')
        for i, v in pairs(caps) do
            Log.info('%s: %s', i, v)
        end

        self:setPreloadParams({
            message = 'INITIALIZING %i%%'
        })
    end

    -- Check for saved games
    local states = GameState.list()
    if #states > 0 then
        GameState.current = self:cache('state:' .. states[1])
    end
end

function ScreenTitle:entered()
    Screen.push('screen.title.menu')
end

function ScreenTitle:update(dt)

end

function ScreenTitle:render()
    self:view():clear()

    if self ~= Screen.currentScreen() then
        self:view():fillFsQuad(0, 0, 0, 230)
    end
end

return ScreenTitle
