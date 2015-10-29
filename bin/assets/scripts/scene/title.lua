local Scene = require 'nx.scene'
local Keyboard = require 'nx.keyboard'
local Mouse = require 'nx.mouse'
local Gamepad = require 'nx.gamepad'
local class = require 'nx.class'
local Renderer = require 'nx.renderer'
local SceneTitle = class('scene.title', Scene)

function SceneTitle:onQuit()
    print('quitting?')
end

function SceneTitle:onFocus(hasFocus)
    print('Window has focus? ' .. tostring(hasFocus))
end

function SceneTitle:onVisible(isVisible)
    print('Window is visible? ' .. tostring(isVisible))
end

function SceneTitle:onTextInput(text)
    print('Text entered: ' .. text)
end

function SceneTitle:onTextEdit(text, start, length)
    print('Text edited from ' .. start .. ' to ' .. (start + length) .. ': ' .. text)
end

function SceneTitle:onMouseFocus(hasFocus)
    print('Has mouse focus? ' .. tostring(hasFocus))
end

function SceneTitle:onMouseDown(x, y, button)
    print('Mouse button pressed at: ' .. x .. ', ' .. y .. ' Button: ' .. button)
end

function SceneTitle:onMouseUp(x, y, button)
    print('Mouse button released at: ' .. x .. ', ' .. y .. ' Button: ' .. button)
end

function SceneTitle:onTouchDown(finger, x, y)
    print('Touch pressed at: ' .. x .. ', ' .. y .. ', finger: ' .. finger)
end

function SceneTitle:onTouchUp(finger, x, y)
    print('Touch released at: ' .. x .. ', ' .. y .. ', finger: ' .. finger)
end

function SceneTitle:onMouseMotion(x, y)
    if Mouse.getRelativeMode() then
        x, y = Mouse.getPosition()
        print(x .. ' ' .. y)
    end
end

function SceneTitle:onKeyDown(scancode, keysym, repeated)
    print('Key pressed: ' .. scancode .. ' sym: ' .. keysym .. ' isRepeat? ' .. tostring(repeated))

    if scancode == 'Backspace' then
        print('Ctrl ' .. tostring(Keyboard.modKeyDown('ctrl')) ..
            '; Alt ' .. tostring(Keyboard.modKeyDown('alt')))
    elseif scancode == 'Return' then
        print('text input: ' .. tostring(Keyboard.isTextInputActive()))
        if (Keyboard.isTextInputActive()) then
            Keyboard.stopTextInput()
        else
            Keyboard.startTextInput(10, 10, 100, 30)
        end
    elseif scancode == 'F1' then
        print(tostring(Mouse.isButtonDown('right')) .. ' ' .. tostring(Mouse.isButtonDown('left')))
    elseif scancode == 'F2' then
        print('relative mode? ' .. tostring(Mouse.getRelativeMode()))
        Mouse.setRelativeMode(not Mouse.getRelativeMode())
    elseif scancode == 'F3' then
        Mouse.setVisible(not Mouse.isVisible())
    elseif scancode == 'F4' then
        Gamepad.loadMappings('assets/gamecontrollerdb.txt')
        Gamepad.saveMappings('gamecontrollerdb.txt')
    elseif scancode == 'F5' then
        local mapping = Gamepad.getMapping('03000000de280000ff11000001000000')
        -- print (mapping)
        for target, data in pairs(mapping) do
            print(target, data.type, data.index, data.hat)
        end
    elseif scancode == 'Space' and Gamepad.isMapped(1) then
        print('Left Stick: ' .. Gamepad.getAxisPosition(1, 'leftx') .. ' ' .. Gamepad.getAxisPosition(1, 'lefty'))
        print('Right Stick: ' .. Gamepad.getAxisPosition(1, 'rightx') .. ' ' .. Gamepad.getAxisPosition(1, 'righty'))
        print('Triggers: ' .. Gamepad.getAxisPosition(1, 'ltrigger') .. ' ' .. Gamepad.getAxisPosition(1, 'rtrigger'))

        print('A: ' .. tostring(Gamepad.isButtonDown(1, 'a')) .. '\t B: ' .. tostring(Gamepad.isButtonDown(1, 'b')))
        print('X: ' .. tostring(Gamepad.isButtonDown(1, 'x')) .. '\t Y: ' .. tostring(Gamepad.isButtonDown(1, 'y')))
        print('Back: ' .. tostring(Gamepad.isButtonDown(1, 'back')) .. '\t Start: ' .. tostring(Gamepad.isButtonDown(1, 'start')))
        print('Up: ' .. tostring(Gamepad.isButtonDown(1, 'up')) .. '\t Down: ' .. tostring(Gamepad.isButtonDown(1, 'down')))
        print('Left: ' .. tostring(Gamepad.isButtonDown(1, 'left')) .. '\t Right: ' .. tostring(Gamepad.isButtonDown(1, 'right')))
    end
end

function SceneTitle:render()
    if Mouse.isButtonDown('left') then
        Renderer.clear(128, 255, 0)
    else
        Renderer.clear(255, 128, 0)
    end
end

return SceneTitle