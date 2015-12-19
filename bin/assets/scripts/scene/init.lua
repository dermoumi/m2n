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
local sceneStack = {}
local releaseStack = {}

------------------------------------------------------------
function Scene.static.currentScene()
    return sceneStack[#sceneStack]
end

------------------------------------------------------------
function Scene.static.goTo(scene, ...)
    for i = #sceneStack, 1, -1 do
        releaseStack[#releaseStack + 1] = sceneStack[i]
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
            return Scene.push('scene.load', scene)
        end
    end

    sceneStack[#sceneStack+1] = scene
    scene:_load()
end

------------------------------------------------------------
function Scene.static.back(...)
    local scene = sceneStack[#sceneStack]
    releaseStack[#releaseStack + 1] = scene

    sceneStack[#sceneStack] = nil

    if sceneStack[#sceneStack] then
        sceneStack[#sceneStack]:back(scene, ...)
    end
end

------------------------------------------------------------
function Scene.static.clean()
    for i, scene in ipairs(releaseStack) do
        scene:_release()
    end

    releaseStack = {}
end

------------------------------------------------------------
function Scene:_load()
    self.parent = sceneStack[#sceneStack-1]

    self.__isLoading = true
    if self:load() then self:performTransition() end
    self.__isLoading = false
end

------------------------------------------------------------
function Scene:_update(dt)
    if self.parent then
        if self:updateParent() then
            self.parent:_update(dt)
        else
            self.parent._transitionTime = nil
        end
    end

    self:updateTransition(dt)
    self:update(dt)
end

------------------------------------------------------------
function Scene:_fixedUpdate(dt)
    if self:updateParent() and self.parent then self.parent:_fixedUpdate(dt) end

    self:fixedUpdate(dt)
end

------------------------------------------------------------
function Scene:_render()
    if self:renderParent() and self.parent then self.parent:_render() end

    self:render()

    -- Render transition if there's any
    self:renderTransition()
end

------------------------------------------------------------
function Scene:_release()
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
function Scene:_onEvent(e, a, b, c, d)
    if self._transitionTime then
        -- While transitioning, disable most events
        return true
    elseif self[e](self, a, b, c, d) == false then
        return false
    elseif self:processParent() and self.parent then
        return self.parent:_onEvent(e, a, b, c, d)
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
function Scene:performTransition(arg1, arg2)
    if self._transitionTime then return end

    if self ~= Scene.currentScene() then
        Scene.currentScene():performTransition(arg1, arg2)
    else
        self._transitionTime = 0

        if type(arg1) == 'function' then
            -- Has a callback, assume it's a fade out transition
            self._transitionFadingIn = true
            self._transitionCallback = arg2 and function() arg1(arg2) end or arg1
        else
            self._transitionFadingIn = false
            self._transitionCallback = nil
        end
    end
end

------------------------------------------------------------
function Scene:updateTransition(dt)
    if not self._transitionTime then return end

    local duration = self:transitionDuration(self._transitionFadingIn)

    if self._transitionTime >= duration then
        if self._transitionCallback then
            self._transitionCallback()
            self._transitionCallback = nil
        else
            self._transitionTime = nil
        end
    else
        self._transitionTime = math.min(duration, self._transitionTime + dt)
    end
end

------------------------------------------------------------
function Scene:renderTransition()
    if not self._transitionTime then return end

    local r, g, b, a = self:transitionColor()

    a = a * (self._transitionTime / self:transitionDuration())
    if not self._transitionFadingIn then a = 255 - a end

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
function Scene:cache(id, loaderFunc)
    -- Make local cache table if there isn't any
    self.__cache = self.__cache or {}

    -- Attempt to load it from the scene's local cache
    local obj = self.__cache[id]
    if obj then return obj end

    -- Not found, attempt to load it from game cache
    obj, err = Cache.get(id, loaderFunc, true)
    if obj then
        self.__cache[id] = obj
        return obj
    end

    -- Not found in game cache, return error
    return nil, err
end

------------------------------------------------------------
function Scene:view()
    if not self._view then
        self._view = Camera2D:new()
    end

    return self._view
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
    if scene._transitionTime ~= nil then
        self:performTransition()
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
    -- Nothing to do
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
