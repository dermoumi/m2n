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

local Cache    = require 'game.cache'
local Worker   = require 'game.worker'
local Camera2D = require 'nx.camera2d'
local class    = require 'nx.class'

local Scene = class 'nx.scene'

-- Local variables -----------------------------------------
local sceneStack, releaseStack = {}, {}

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

------------------------------------------------------------
function Scene.static.currentScene()
    return sceneStack[#sceneStack]
end

------------------------------------------------------------
function Scene.static.goTo(scene, ...)
    for i = #sceneStack, 1, -1 do
        releaseStack[#releaseStack+1] = sceneStack[i]
        sceneStack[i] = nil
    end

    Scene.push(scene, ...)
end

------------------------------------------------------------
function Scene.static.push(scene, ...)
    if type(scene) == 'string' then
        scene = require(scene):new(...)

        -- Check if this scene's worker contains any item that need preloading
        if scene.__worker and scene.__worker:taskCount() > 0 then
            -- Start preloading
            return Scene.push('scene._load', scene)
        end
    end

    sceneStack[#sceneStack+1] = scene
    scene:__load()
end

------------------------------------------------------------
function Scene.static.back(...)
    local scene = sceneStack[#sceneStack]
    releaseStack[#releaseStack+1] = scene

    sceneStack[#sceneStack] = nil

    if sceneStack[#sceneStack] then
        sceneStack[#sceneStack]:back(scene, ...)
    end
end

------------------------------------------------------------
function Scene.static.replace(scene, ...)
    local lastScene = sceneStack[#sceneStack]
    releaseStack[#releaseStack+1] = lastScene

    sceneStack[#sceneStack] = nil

    Scene.push(scene, ...)
end

------------------------------------------------------------
function Scene.static.clean()
    for i, scene in ipairs(releaseStack) do
        scene:__release()
    end

    releaseStack = {}
end

------------------------------------------------------------
function Scene:__load()
    self.parent = sceneStack[#sceneStack-1]

    self.__isLoading = true
    self:load()
    self.__isLoading = false

    if not self.parent or self.parent:isTransitioning() then
        self:performTransition()
    end
end

------------------------------------------------------------
function Scene:__update(dt)
    if self.parent and self:updateParent() then
        self.parent:__update(dt)
    end

    self:updateTransition(dt)
    self:update(dt)
end

------------------------------------------------------------
function Scene:__fixedUpdate(dt)
    if self.parent and self:updateParent() then
        self.parent:__fixedUpdate(dt)
    end

    self:fixedUpdate(dt)
end

------------------------------------------------------------
function Scene:__render()
    if self.parent and self:renderParent() then
        self.parent:__render()
    end

    self:render()
    self:renderTransition()
end

------------------------------------------------------------
function Scene:__release()
    -- Release all cached elements
    if self.__cache then
        for i, v in pairs(self.__cache) do
            Cache.release(i)
        end
    end

    -- Call the scene's release method
    self:release()
end

------------------------------------------------------------
function Scene:__onEvent(e, a, b, c, d)
    if not self:isTransitioning() then
        if self:onEvent(e, a, b, c, d) == false then
            return false
        elseif self:updateParent() and self.parent then
            return self.parent:__onEvent(e, a, b, c, d)
        end
    end

    return true
end

------------------------------------------------------------
function Scene:worker()
    if not self.__worker then self.__worker = Worker:new() end
    return self.__worker
end

------------------------------------------------------------
function Scene:setPreloadParams(table)
    self.__preloadParams = table
end

------------------------------------------------------------
function Scene:isLoading()
    return self.__isLoading
end

------------------------------------------------------------
function Scene:performTransition(callback, arg)
    if self.__transitionTime then return end

    if self ~= Scene.currentScene() then
        Scene.currentScene():performTransition(callback, arg)
    else
        self.__transitionTime = 0

        if type(callback) == 'function' then
            -- Has a callback, assume it's a fade out transition
            self.__transitionFadingIn = true
            self.__transitionCallback = callback
            self.__transitionCallbackArg = arg
        else
            self.__transitionFadingIn = false
            self.__transitionCallback = nil
            self.__transitionCallbackArg = nil
        end
    end
end

------------------------------------------------------------
function Scene:updateTransition(dt)
    if not self.__transitionTime then return end

    local duration = self:transitionDuration(self.__transitionFadingIn)

    if self.__transitionTime >= duration then
        if self.__transitionCallback then
            self.__transitionCallback(self.__transitionCallbackArg)
            self.__transitionCallback = nil
            self.__transitionCallbackArg = nil
        else
            self.__transitionTime = nil
        end
    else
        self.__transitionTime = math.min(duration, self.__transitionTime + dt)
    end
end

------------------------------------------------------------
function Scene:renderTransition()
    if not self.__transitionTime then return end

    local r, g, b, a = self:transitionColor()

    a = a * (self.__transitionTime / self:transitionDuration())
    if not self.__transitionFadingIn then a = 255 - a end

    self:view():fillFsQuad(r, g, b, a)
end

------------------------------------------------------------
function Scene:transitionColor()
    return 0, 0, 0, 255
end

------------------------------------------------------------
function Scene:transitionDuration()
    return .2
end

------------------------------------------------------------
function Scene:isTransitioning()
    return self.__transitionTime ~= nil
end

------------------------------------------------------------
function Scene:cache(id, loaderFunc)
    -- Make local cache table if there isn't any
    self.__cache = self.__cache or {}

    -- Attempt to load it from the scene's local cache
    local obj = self.__cache[id]
    if not obj then
        -- Not found, attempt to load it from game cache
        obj, err = Cache.get(id, loaderFunc, true)
        if not obj then return nil, err end

        self.__cache[id] = obj
    end

    return obj
end

------------------------------------------------------------
function Scene:view()
    if not self.__view then
        self.__view = Camera2D:new()
    end

    return self.__view
end

------------------------------------------------------------
function Scene:load()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:update(dt)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:fixedUpdate(dt)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:render()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:back(scene, ...)
    if scene:isTransitioning() then
        self:performTransition()
    else
        self.__transitionTime = self:transitionDuration()
    end
end

------------------------------------------------------------
function Scene:release()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onQuit()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onFocus(hasFocus)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onVisible(isVisible)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onResize(w, h)
    self:view():reset(0, 0, w, h)
end

------------------------------------------------------------
function Scene:onTextInput(text)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTextEdit(text, start, length)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseFocus(hasFocus)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onKeyDown(scancode, keysym, repeated)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onKeyUp(scancode, keysym)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseMotion(x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseDown(x, y, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseUp(x, y, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onWheelScroll(x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyAxisMotion(which, axis, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyBallMotion(which, ball, xrel, yrel)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyHatMotion(which, hat, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyButtonDown(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyButtonUp(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyConnect(which, isConnected)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadMotion(which, axis, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadButtonDown(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadButtonUp(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadConnect(which, isConnected)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadRemap(which)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchDown(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchUp(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchMotion(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onFileDrop(file)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onEvent(e, a, b, c, d)
    local event = eventMapping[e]
    if not event then return true end

    return self[event](self, a, b, c, d)
end

------------------------------------------------------------
function Scene:processParent()
    return false
end

------------------------------------------------------------
function Scene:updateParent()
    return self:processParent()
end

------------------------------------------------------------
function Scene:renderParent()
    return self:processParent()
end

------------------------------------------------------------
return Scene
