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
-- A Title scene, probably the first scene the user sees
------------------------------------------------------------
local class = require 'nx.class'
local Scene = require 'scene'
local SceneTitle = class('scene.title', Scene)

------------------------------------------------------------
local Mouse = require 'nx.mouse'
local Renderer = require 'nx.renderer'

------------------------------------------------------------
function SceneTitle:load()
    Scene.push('scene.subtitle')
end

------------------------------------------------------------
function SceneTitle:render()
    if Mouse.isButtonDown('left') then
        Renderer.clear(128, 255, 0)
    else
        Renderer.clear(255, 128, 0)
    end
end

------------------------------------------------------------
function SceneTitle:onResize(w, h)
    Renderer.setupViewport(0, 0, w, h)
end

------------------------------------------------------------
function SceneTitle:onKeyDown(scancode)
    if scancode == 'f1' then
        Scene.push('scene.subtitle')
    elseif scancode == 'f10' then
        require('nx.window').create('m2n-', 1280, 720, {})
    elseif scancode == 'f11' then
        require('nx.window').create('m2n-', 1920, 1080, {fullscreen = true, msaa = 4})
    elseif scancode == 'f12' then
        require('nx.window').create('m2n-', 1024, 720, {fullscreen = true, msaa = 8})
    end
end

------------------------------------------------------------
return SceneTitle