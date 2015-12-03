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
-- A base class for the different scenes in the game
------------------------------------------------------------
local class = require 'nx.class'
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
    if type(scene) == 'string' then scene = require(scene):new(...) end
    scene.parent = sceneStack[#sceneStack]

    local needsPreload, settings = scene:needsPreload()
    if needsPreload and not scene._preloaded then
        Scene.push('scene.load', settings, scene)
    else
        sceneStack[#sceneStack + 1] = scene

        scene._isLoading = true
        scene:load()
        scene._isLoading = false
    end
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
        scene:release()
    end
    releaseStack = {}
end

------------------------------------------------------------
function Scene:_update(dt)
    if self:updateParent() and self.parent then self.parent:_update(dt) end

    if self._transitionTime then self:updateTransition(dt) end
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
    if self._transitionTime then
        local camera = self._transitionCamera
        local time = self._transitionTime
        local duration = self:transitionDuration()
        local fadeIn = self._transitionFadingIn

        self:renderTransition(camera, time, duration, fadeIn)
    end
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
function Scene:needsPreload()
    return false
end

------------------------------------------------------------
function Scene:updateTransition(dt)
    local duration = self:transitionDuration(self._transitionFadingIn)

    if self._transitionTime >= duration then
        self._transitionTime = nil
    else
        self._transitionTime = math.min(duration, self._transitionTime + dt)
        if self._transitionTime >= duration and self._transitionCallback then
            self._transitionCallback()
        end
    end
end

------------------------------------------------------------
function Scene:performTransition(camera, arg)
    if self._transitionTime then return end

    local currentScene = Scene.currentScene()
    if self == currentScene then
        self._transitionTime = 0
        self._transitionCamera = camera

        if type(arg) == 'function' then
            -- Has a callback, assume it's a fade out transition
            self._transitionFadingIn = true
            self._transitionCallback = arg
        else
            self._transitionFadingIn = false
            self._transitionCallback = nil
        end
    else
        currentScene:performTransition(camera, arg)
    end
end

------------------------------------------------------------
function Scene:renderTransition(camera, time, duration, fadingIn)
    local r, g, b, a = self:transitionColor()

    a = a * (time / duration)
    if not fadingIn then a = 255 - a end

    camera:fillFsQuad(r, g, b, a)
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
function Scene:preload(worker)
    -- Nothing to do
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
function Scene:back(...)
    -- Nothing to do
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
    -- Nothing to do
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
function Scene:onKeyUp(scancode, keysym, repeated)
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
    return true
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
