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

local ffi        = require 'ffi'
local class      = require 'nx.class'
local Quaternion = require 'nx.util.quaternion'

local Matrix = class 'nx.util.matrix'

------------------------------------------------------------
local EPSILON = 0.000001

------------------------------------------------------------
function Matrix.static.fromTranslation(x, y, z)
    local mat = Matrix:new()
    local m   = mat._cdata

    m[12] = x
    m[13] = y
    m[14] = z

    return mat
end

------------------------------------------------------------
function Matrix.static.fromScaling(x, y, z)
    local mat = Matrix:new()
    local m   = mat._cdata

    m[0]  = x
    m[5]  = y
    m[10] = z

    return mat
end

------------------------------------------------------------
function Matrix.static.fromQuaternion(x, y, z, w)
    -- Get quaternion
    if not y then
        x, y, z, w = x.x, x.y, x.z, x.w
    end

    -- Calculate coefficients
    local x2, y2, z2 = x+x,  y+y,  z+z
    local xx, xy, xz = x*x2, x*y2, x*z2
    local yy, yz, zz = y*y2, y*z2, z*z2
    local wx, wy, wz = w*x2, w*y2, w*z2

    local mat = Matrix:new()
    local m = mat._cdata

    m[0]  = 1 - yy - zz
    m[1]  = xy + wz
    m[2]  = xz - wy
    m[4]  = xy - wz
    m[5]  = 1 - xx - zz
    m[6]  = yz + wx
    m[8]  = xz + wy
    m[9]  = yz - wx
    m[10] = 1 - xx - yy

    return mat
end

------------------------------------------------------------
function Matrix.static.fromRotation(x, y, z)
    return Matrix.fromQuaternion(Quaternion:new(x, y, z))
end

------------------------------------------------------------
function Matrix.static.fromTransformation(quat, posX, posY, posZ, sx, sy, sz)
    local x, y, z, w = quat.x, quat.y, quat.z, quat.w

    -- Calculate coefficients
    local x2, y2, z2 = x+x,  y+y,  z+z
    local xx, xy, xz = x*x2, x*y2, x*z2
    local yy, yz, zz = y*y2, y*z2, z*z2
    local wx, wy, wz = w*x2, w*y2, w*z2

    local mat = Matrix:new()
    local m = mat._cdata

    m[0]  = sx * (1 - yy - zz)
    m[1]  = sx * (xy + wz)
    m[2]  = sx * (xz - wy)
    m[4]  = sy * (xy - wz)
    m[5]  = sy * (1 - xx - zz)
    m[6]  = sy * (yz + wx)
    m[8]  = sz * (xz + wy)
    m[9]  = sz * (yz - wx)
    m[10] = sz * (1 - xx - yy)
    m[12] = posX
    m[13] = posY
    m[14] = posZ

    return mat
end

------------------------------------------------------------
function Matrix.static.fromFrustum(left, right, bottom, top, near, far)
    local mat = Matrix:new()
    local m   = mat._cdata

    local n2 = 2 * near
    local rl = right - left
    local tb = top - bottom
    local fn = far - near

    m[0]  = n2 / rl
    m[5]  = n2 / tb
    m[8]  = (right + left) / rl
    m[9]  = (top + bottom) / tb
    m[10] = -(far + near) / fn
    m[11] = -1
    m[14] = -n2 * far / fn
    m[15] = 0

    return mat
end

------------------------------------------------------------
function Matrix.static.fromOrtho(left, right, bottom, top, near, far)
    local mat = Matrix:new()
    local m   = mat._cdata

    m[0]  = 2 / (right - left)
    m[5]  = 2 / (top - bottom)
    m[10] = -2 / (far - near)
    m[12] = -(right + left)   / (right - left)
    m[13] = -(top   + bottom) / (top   - bottom)
    m[14] = -(far   + near)   / (far   - near)

    return mat
end

------------------------------------------------------------
function Matrix.static.fastMult43(mat1, mat2)
    local mat = Matrix:new()
    local m, m1, m2 = mat._cdata, mat1._cdata, mat2._cdata

    local a00, a01, a02, a03 = m1[0],  m1[1],  m1[2],  m1[3]
    local a10, a11, a12, a13 = m1[4],  m1[5],  m1[6],  m1[7]
    local a20, a21, a22, a23 = m1[8],  m1[9],  m1[10], m1[11]
    local a30, a31, a32, a33 = m1[12], m1[13], m1[14], m1[15]

    -- Cache only the current line of the second matrix
    local b0, b1, b2, b3 = m2[0], m2[1], m2[2], m2[3]
    m[0]  = b0*a00 + b1*a10 + b2*a20
    m[1]  = b0*a01 + b1*a11 + b2*a21
    m[2]  = b0*a02 + b1*a12 + b2*a22

    b0, b1, b2, b3 = m2[4], m2[5], m2[6], m2[7]
    m[4]  = b0*a00 + b1*a10 + b2*a20
    m[5]  = b0*a01 + b1*a11 + b2*a21
    m[6]  = b0*a02 + b1*a12 + b2*a22

    b0, b1, b2, b3 = m2[8], m2[9], m2[10], m2[11]
    m[8]  = b0*a00 + b1*a10 + b2*a20
    m[9]  = b0*a01 + b1*a11 + b2*a21
    m[10] = b0*a02 + b1*a12 + b2*a22

    b0, b1, b2, b3 = m2[12], m2[13], m2[14], m2[15]
    m[12] = b0*a00 + b1*a10 + b2*a20 + b3*a30
    m[13] = b0*a01 + b1*a11 + b2*a21 + b3*a31
    m[14] = b0*a02 + b1*a12 + b2*a22 + b3*a32

    return mat
end

------------------------------------------------------------
function Matrix:initialize(mat)
    self._cdata = ffi.new('float[16]', mat and mat._cdata or {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    })
end

------------------------------------------------------------
function Matrix:combine(mat)
    local m1, m2 = self._cdata, mat._cdata

    local a00, a01, a02, a03 = m1[0],  m1[1],  m1[2],  m1[3]
    local a10, a11, a12, a13 = m1[4],  m1[5],  m1[6],  m1[7]
    local a20, a21, a22, a23 = m1[8],  m1[9],  m1[10], m1[11]
    local a30, a31, a32, a33 = m1[12], m1[13], m1[14], m1[15]

    -- Cache only the current line of the second matrix
    local b0, b1, b2, b3 = m2[0], m2[1], m2[2], m2[3]
    m1[0]  = b0*a00 + b1*a10 + b2*a20 + b3*a30
    m1[1]  = b0*a01 + b1*a11 + b2*a21 + b3*a31
    m1[2]  = b0*a02 + b1*a12 + b2*a22 + b3*a32
    m1[3]  = b0*a03 + b1*a13 + b2*a23 + b3*a33

    b0, b1, b2, b3 = m2[4], m2[5], m2[6], m2[7]
    m1[4]  = b0*a00 + b1*a10 + b2*a20 + b3*a30
    m1[5]  = b0*a01 + b1*a11 + b2*a21 + b3*a31
    m1[6]  = b0*a02 + b1*a12 + b2*a22 + b3*a32
    m1[7]  = b0*a03 + b1*a13 + b2*a23 + b3*a33

    b0, b1, b2, b3 = m2[8], m2[9], m2[10], m2[11]
    m1[8]  = b0*a00 + b1*a10 + b2*a20 + b3*a30
    m1[9]  = b0*a01 + b1*a11 + b2*a21 + b3*a31
    m1[10] = b0*a02 + b1*a12 + b2*a22 + b3*a32
    m1[11] = b0*a03 + b1*a13 + b2*a23 + b3*a33

    b0, b1, b2, b3 = m2[12], m2[13], m2[14], m2[15]
    m1[12] = b0*a00 + b1*a10 + b2*a20 + b3*a30
    m1[13] = b0*a01 + b1*a11 + b2*a21 + b3*a31
    m1[14] = b0*a02 + b1*a12 + b2*a22 + b3*a32
    m1[15] = b0*a03 + b1*a13 + b2*a23 + b3*a33

    return self
end

------------------------------------------------------------
function Matrix:inverse()
    local m = self._cdata

    local a00, a01, a02, a03 = m[0],  m[1],  m[2],  m[3]
    local a10, a11, a12, a13 = m[4],  m[5],  m[6],  m[7]
    local a20, a21, a22, a23 = m[8],  m[9],  m[10], m[11]
    local a30, a31, a32, a33 = m[12], m[13], m[14], m[15]

    local b00 = a00 * a11 - a01 * a10
    local b01 = a00 * a12 - a02 * a10
    local b02 = a00 * a13 - a03 * a10
    local b03 = a01 * a12 - a02 * a11
    local b04 = a01 * a13 - a03 * a11
    local b05 = a02 * a13 - a03 * a12
    local b06 = a20 * a31 - a21 * a30
    local b07 = a20 * a32 - a22 * a30
    local b08 = a20 * a33 - a23 * a30
    local b09 = a21 * a32 - a22 * a31
    local b10 = a21 * a33 - a23 * a31
    local b11 = a22 * a33 - a23 * a32

    local det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06
    if det == 0 then return nil end

    local mat = Matrix:allocate()
    mat._cdata = ffi.new('float[16]', {
        (a11 * b11 - a12 * b10 + a13 * b09) / det,
        (a02 * b10 - a01 * b11 - a03 * b09) / det,
        (a31 * b05 - a32 * b04 + a33 * b03) / det,
        (a22 * b04 - a21 * b05 - a23 * b03) / det,
        (a12 * b08 - a10 * b11 - a13 * b07) / det,
        (a00 * b11 - a02 * b08 + a03 * b07) / det,
        (a32 * b02 - a30 * b05 - a33 * b01) / det,
        (a20 * b05 - a22 * b02 + a23 * b01) / det,
        (a10 * b10 - a11 * b08 + a13 * b06) / det,
        (a01 * b08 - a00 * b10 - a03 * b06) / det,
        (a30 * b04 - a31 * b02 + a33 * b00) / det,
        (a21 * b02 - a20 * b04 - a23 * b00) / det,
        (a11 * b07 - a10 * b09 - a12 * b06) / det,
        (a00 * b09 - a01 * b07 + a02 * b06) / det,
        (a31 * b01 - a30 * b03 - a32 * b00) / det,
        (a20 * b03 - a21 * b01 + a22 * b00) / det
    })

    return mat
end

------------------------------------------------------------
function Matrix:apply(x, y, z)
    local m = self._cdata

    local w = m[3] * x + m[7] * y + m[11] * z + m[15]
    if w == 0 then w = 1 end

    local outX = (m[0] * x + m[4] * y + m[8]  * z + m[12]) / w
    local outY = (m[1] * x + m[5] * y + m[9]  * z + m[13]) / w
    local outZ = (m[2] * x + m[6] * y + m[10] * z + m[14]) / w

    return outX, outY, outZ
end

------------------------------------------------------------
function Matrix:data()
    return self._cdata
end

------------------------------------------------------------
return Matrix