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

local class = require 'class'
local Cache = require 'game.cache'
local View  = require 'graphics.view'

local Screen = class 'screen'

-- Local variables
local screenStack, lastScreen = {}, nil

local function addScreen(screen, ...)
    if type(screen) == 'string' then
        screen = require(screen):new(...)

        -- Check if the cache contains any item that need preloading
        if Cache.hasTasks() and screen.class.name ~= 'screen._load' then
            -- Start preloading
            Screen.push('screen._load', screen)
            return
        end
    end

    screenStack[#screenStack+1] = screen
    screen:__entered()
end

local function dropScreen()
    lastScreen = screenStack[#screenStack]
    lastScreen:__left()

    screenStack[#screenStack] = nil
end

function Screen.static.currentScreen()
    return screenStack[#screenStack]
end

function Screen.static.lastScreen()
    return lastScreen
end

function Screen.static.goTo(screen, ...)
    lastScreen = Screen.currentScreen()

    for i = #screenStack, 1, -1 do
        screenStack[i]:__left()
        screenStack[i] = nil
    end

    addScreen(screen, ...)
end

function Screen.static.push(screen, ...)
    lastScreen = Screen.currentScreen()

    addScreen(screen, ...)
end

function Screen.static.back(...)
    dropScreen()

    if screenStack[#screenStack] then
        screenStack[#screenStack]:returned(...)
    end
end

function Screen.static.replace(screen, ...)
    dropScreen()

    addScreen(screen, ...)
end

function Screen:__entered()
    self.parent = screenStack[#screenStack-1]

    self.__isLoading = true
    self:entered()
    if self.resized then self:resized(require('window').size()) end
    self.__isLoading = false

    if not Screen.lastScreen() or Screen.lastScreen():isTransitioning() then
        self:performTransition()
    end
end

function Screen:__update(dt)
    if self.parent and self:updateParent() then
        self.parent:__update(dt)
    end

    if self:isTransitioning() then
        self:updateTransition(dt)
    end

    self:update(dt)
end

function Screen:__fixedUpdate(dt)
    if self.parent and self:updateParent() then
        self.parent:__fixedUpdate(dt)
    end

    self:fixedUpdate(dt)
end

function Screen:__render()
    if self.parent and self:renderParent() then
        self.parent:__render()
    end

    self:render()

    if self:isTransitioning() then
        self:renderTransition(self.__transTime, self.__opening)
    end
end

function Screen:__left()
    -- Call the screen's release method
    self:left()

    -- Release all cached elements
    if self.__cache then
        for i in pairs(self.__cache) do
            Cache.release(i)
        end
    end
end

function Screen:__onEvent(e, a, b, c, d)
    if not self:isTransitioning() then
        if self:onEvent(e, a, b, c, d) == false then
            return false
        elseif self:bubbleEvents() and self.parent then
            return self.parent:__onEvent(e, a, b, c, d)
        end
    end

    return true
end

function Screen:setPreloadParams(table)
    self.__preloadParams = table
end

function Screen:isLoading()
    return self.__isLoading
end

function Screen:performTransition(callback, arg)
    if self ~= Screen.currentScreen() then
        Screen.currentScreen():performTransition(callback, arg)
    elseif not self:isTransitioning() then
        self.__transTime = 0

        if callback then
            -- Has a callback, assume it's a fade out transition
            self.__opening = false
            self.__transCb = arg and function() callback(arg) end or callback
        else
            self.__opening = true
            self.__transCb = nil
        end
    end
end

function Screen:updateTransition(dt)
    local duration = self:transitionDuration()

    if self.__transTime >= duration then
        if self.__transCb then
            self.__transCb()
            self.__transCb = nil
        end

        self.__transTime = nil
    else
        self.__transTime = math.min(duration, self.__transTime + dt)
    end
end

function Screen:renderTransition(time, opening)
    local r, g, b, a = self:transitionColor()

    a = a * (time / self:transitionDuration())
    if opening then a = 255 - a end

    self:view():fillFsQuad(r, g, b, a)
end

function Screen:transitionColor()
    return 0, 0, 0, 255
end

function Screen:transitionDuration()
    return .2
end

function Screen:isTransitioning()
    return self.__transTime
end

function Screen:isOpening()
    return self:isTransitioning() and self.__opening
end

function Screen:isClosing()
    return self:isTransitioning() and not self.__opening
end

function Screen:cache(id, peek)
    -- Make local cache table if there isn't any
    self.__cache = self.__cache or {}

    -- Attempt to load it from the screen's local cache
    local item = self.__cache[id]
    if not item then
        -- Not found, attempt to load it from game cache
        obj, reusable = Cache.get(self, id, peek)

        if reusable then
            item = {
                obj = obj,
                count = 0
            }
            self.__cache[id] = item
        else
            return obj, false
        end
    end

    if not peek then
        item.count = item.count + 1
    end

    return item.obj, true
end

function Screen:uncache(id)
    if not self.__cache then return end

    local item = self.__cache[id]
    if item then
        item.count = item.count - 1
        if item.count <= 0 then
            Cache.release(id)
        end
    end
end

function Screen:view()
    if not self.__view then
        self.__view = View:new()
    end

    return self.__view
end

function Screen:entered()
    -- Nothing to do
end

function Screen:update(dt)
    -- Nothing to do
end

function Screen:fixedUpdate(dt)
    -- Nothing to do
end

function Screen:render()
    -- Nothing to do
end

function Screen:returned(...)
    if Screen.lastScreen() and Screen.lastScreen():isTransitioning() then
        self:performTransition()
    else
        self.__transTime = self:transitionDuration()
    end
end

function Screen:left()
    -- Nothing to do
end

function Screen:onEvent(e, a, b, c, d)
    local eventFunc = self[e]
    if not eventFunc then return true end

    return eventFunc(self, a, b, c, d)
end

function Screen:resized(width, height)
    self:view()
        :reset(0, 0, width, height)
        :setViewport(0, 0, width, height)
end

function Screen:processParent()
    return false
end

function Screen:updateParent()
    return self:processParent()
end

function Screen:renderParent()
    return self:processParent()
end

function Screen:bubbleEvents()
    return false
end

return Screen
