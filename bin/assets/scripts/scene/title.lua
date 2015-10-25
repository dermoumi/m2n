local Scene = require 'nx.scene'
local Keyboard = require 'nx.keyboard'
local Mouse = require 'nx.mouse'
local Joystick = require 'nx.joystick'
local class = require 'nx.class'

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
    elseif scancode == 'Space' and Joystick.isConnected(1) then
        for i=1, Joystick.getButtonCount(1) do
            print('Button ' .. i .. ' is: ' .. tostring(Joystick.isButtonDown(1, i)))
        end
        for i=1, Joystick.getAxisCount(1) do
            print('Axis ' .. i .. ' is: ' .. Joystick.getAxisPosition(1, i))
        end
        for i=1, Joystick.getBallCount(1) do
            local x, y = Joystick.getBallPosition(1, i)
            print('Ball ' .. i .. ' is: ' .. x .. ', ' .. y)
        end
        for i=1, Joystick.getHatCount(1) do
            print('Hat ' .. i .. ' is: ' .. Joystick.getHatPosition(1, i))
        end
    end
end

return SceneTitle