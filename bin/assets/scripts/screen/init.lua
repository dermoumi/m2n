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

local class    = require 'class'
local Cache    = require 'game.cache'
local Worker   = require 'game.worker'
local Camera2D = require 'graphics.camera2d'

local Screen = class 'screen'

-- Local variables
local screenStack, lastScreen = {}, nil

-- Mapping events to their respective functions
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

local function addScreen(screen, ...)
    if type(screen) == 'string' then
        screen = require(screen):new(...)

        -- Check if this screen's worker contains any item that need preloading
        if screen.__worker and screen.__worker:taskCount() > 0 then
            -- Start preloading
            Screen.push('screen._load', screen)
            return
        end
    end

    screenStack[#screenStack+1] = screen
    screen:__load()
end

local function dropScreen()
    lastScreen = screenStack[#screenStack]
    lastScreen:__release()

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
        screenStack[i]:__release()
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
        screenStack[#screenStack]:back(...)
    end
end

function Screen.static.replace(screen, ...)
    dropScreen()

    addScreen(screen, ...)
end

function Screen:__load()
    self.parent = screenStack[#screenStack-1]

    self.__isLoading = true
    self:load()
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

function Screen:__release()
    -- Call the screen's release method
    self:release()

    -- Release all cached elements
    if self.__cache then
        for i, v in pairs(self.__cache) do
            Cache.release(i)
        end
    end
end

function Screen:__onEvent(e, a, b, c, d)
    if not self:isTransitioning() then
        if self:onEvent(e, a, b, c, d) == false then
            return false
        elseif self:updateParent() and self.parent then
            return self.parent:__onEvent(e, a, b, c, d)
        end
    end

    return true
end

function Screen:worker()
    if not self.__worker then self.__worker = Worker:new() end
    return self.__worker
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

function Screen:cache(id, loaderFunc)
    -- Make local cache table if there isn't any
    self.__cache = self.__cache or {}

    -- Attempt to load it from the screen's local cache
    local obj = self.__cache[id]
    if not obj then
        -- Not found, attempt to load it from game cache
        obj, err = Cache.get(id, loaderFunc, true)
        if not obj then return nil, err end

        self.__cache[id] = obj
    end

    return obj
end

function Screen:view()
    if not self.__view then
        self.__view = Camera2D:new()
    end

    return self.__view
end

function Screen:load()
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

function Screen:back(...)
    if Screen.lastScreen() and Screen.lastScreen():isTransitioning() then
        self:performTransition()
    else
        self.__transTime = self:transitionDuration()
    end
end

function Screen:release()
    -- Nothing to do
end

function Screen:onQuit()
    -- Nothing to do
end

function Screen:onFocus(hasFocus)
    -- Nothing to do
end

function Screen:onVisible(isVisible)
    -- Nothing to do
end

function Screen:onResize(w, h)
    self:view():reset(0, 0, w, h)
end

function Screen:onTextInput(text)
    -- Nothing to do
end

function Screen:onTextEdit(text, start, length)
    -- Nothing to do
end

function Screen:onMouseFocus(hasFocus)
    -- Nothing to do
end

function Screen:onKeyDown(scancode, keysym, repeated)
    -- Nothing to do
end

function Screen:onKeyUp(scancode, keysym)
    -- Nothing to do
end

function Screen:onMouseMotion(x, y, xRel, yRel)
    -- Nothing to do
end

function Screen:onMouseDown(x, y, button)
    -- Nothing to do
end

function Screen:onMouseUp(x, y, button)
    -- Nothing to do
end

function Screen:onWheelScroll(x, y)
    -- Nothing to do
end

function Screen:onJoyAxisMotion(which, axis, value)
    -- Nothing to do
end

function Screen:onJoyBallMotion(which, ball, xrel, yrel)
    -- Nothing to do
end

function Screen:onJoyHatMotion(which, hat, value)
    -- Nothing to do
end

function Screen:onJoyButtonDown(which, button)
    -- Nothing to do
end

function Screen:onJoyButtonUp(which, button)
    -- Nothing to do
end

function Screen:onJoyConnect(which, isConnected)
    -- Nothing to do
end

function Screen:onGamepadMotion(which, axis, value)
    -- Nothing to do
end

function Screen:onGamepadButtonDown(which, button)
    -- Nothing to do
end

function Screen:onGamepadButtonUp(which, button)
    -- Nothing to do
end

function Screen:onGamepadConnect(which, isConnected)
    -- Nothing to do
end

function Screen:onGamepadRemap(which)
    -- Nothing to do
end

function Screen:onTouchDown(finger, x, y)
    -- Nothing to do
end

function Screen:onTouchUp(finger, x, y)
    -- Nothing to do
end

function Screen:onTouchMotion(finger, x, y)
    -- Nothing to do
end

function Screen:onFileDrop(file)
    -- Nothing to do
end

function Screen:onEvent(e, a, b, c, d)
    local event = eventMapping[e]
    if not event then return true end

    return self[event](self, a, b, c, d)
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

return Screen
