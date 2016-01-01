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

local Entity3D = require 'nx.graphics.entity3d'
local class    = require 'nx.class'

local ModelNode = class 'nx.graphics.modelnode'
ModelNode:include(Entity3D)

------------------------------------------------------------
function ModelNode:initialize(model)
    Entity3D.initialize(self)
    self:setModel(model)
end

------------------------------------------------------------
function ModelNode:setModel(model)
    self._model = model
    return self
end

------------------------------------------------------------
function ModelNode:model()
    return self._model
end

------------------------------------------------------------
function ModelNode:_render(camera, context)
    if self._model then
        self._model:_draw(camera:projection(), self:matrix(true), context)
    end
end

------------------------------------------------------------
return ModelNode