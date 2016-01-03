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

local Node = class 'graphics.node'

function Node:initialize()
    self.parent, self.children = nil, {}
end

function Node:attachTo(name, node)
    node:attach(name, self)
    return self
end

function Node:attach(name, node)
    if node.parent ~= self and self:canAttach(node) then
        node:detach()

        if self.children[name] then self.children[name]:detach() end

        self.children[name] = node
        node.parent = self

        self:attached(node)
        node:attachedTo(self)
    end

    return self
end

function Node:detach()
    if self.parent then
        local parent = self.parent
        for name, child in pairs(parent.children) do
            if child == self then
                parent.children[name] = nil
                parent:detached(self)
                break
            end
        end

        self.parent = nil
        self:detachedFrom(parent)
    end

    return self
end

function Node:canAttach(node)
    return true
end

function Node:attached(node)
    -- Nothing to do
end

function Node:attachedTo(node)
    -- Nothing to do
end

function Node:detached(node)
    -- Nothing to do
end

function Node:detachedFrom(node)
    -- Nothing to do
end

function Node:resolveName(name)
    local currNode = self
    for nodeName in string.gmatch(name, '.+') do
        currNode = currNode.children[nodeName]
        if not currNode then return nil end
    end
    return currNode
end

return Node
