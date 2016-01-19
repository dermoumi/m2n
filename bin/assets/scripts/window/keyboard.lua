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

local Keyboard = {}

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    uint32_t nxKeyboardToKeysym(uint32_t scancode);
    uint32_t nxKeyboardToScancode(uint32_t keysym);
    bool nxKeyboardModKeyDown(uint16_t mod);
    void nxKeyboardStartTextInput(int x, int y, int w, int h);
    void nxKeyboardStopTextInput();
    bool nxKeyboardTextInputActive();
]]


-- Scancodes table
local fromScancode = {
    [0] = 'key_unknown',
    [4] = 'a',
    [5] = 'b',
    [6] = 'c',
    [7] = 'd',
    [8] = 'e',
    [9] = 'f',
    [10] = 'g',
    [11] = 'h',
    [12] = 'i',
    [13] = 'j',
    [14] = 'k',
    [15] = 'l',
    [16] = 'm',
    [17] = 'n',
    [18] = 'o',
    [19] = 'p',
    [20] = 'q',
    [21] = 'r',
    [22] = 's',
    [23] = 't',
    [24] = 'u',
    [25] = 'v',
    [26] = 'w',
    [27] = 'x',
    [28] = 'y',
    [29] = 'z',
    [30] = '1',
    [31] = '2',
    [32] = '3',
    [33] = '4',
    [34] = '5',
    [35] = '6',
    [36] = '7',
    [37] = '8',
    [38] = '9',
    [39] = '0',
    [40] = 'return',
    [41] = 'escape',
    [42] = 'backspace',
    [43] = 'tab',
    [44] = 'space',
    [45] = '-',
    [46] = '=',
    [47] = '[',
    [48] = ']',
    [49] = '\\',
    [50] = '#',
    [51] = ';',
    [52] = '\'',
    [53] = '`',
    [54] = ',',
    [55] = '.',
    [56] = '/',
    [57] = 'capslock',
    [58] = 'f1',
    [59] = 'f2',
    [60] = 'f3',
    [61] = 'f4',
    [62] = 'f5',
    [63] = 'f6',
    [64] = 'f7',
    [65] = 'f8',
    [66] = 'f9',
    [67] = 'f10',
    [68] = 'f11',
    [69] = 'f12',
    [70] = 'printscreen',
    [71] = 'scrolllock',
    [72] = 'pause',
    [73] = 'insert',
    [74] = 'home',
    [75] = 'pageup',
    [76] = 'delete',
    [77] = 'end',
    [78] = 'pagedown',
    [79] = 'right',
    [80] = 'left',
    [81] = 'down',
    [82] = 'up',
    [83] = 'numlock',
    [84] = 'keypad /',
    [85] = 'keypad *',
    [86] = 'keypad -',
    [87] = 'keypad +',
    [88] = 'keypad enter',
    [89] = 'keypad 1',
    [90] = 'keypad 2',
    [91] = 'keypad 3',
    [92] = 'keypad 4',
    [93] = 'keypad 5',
    [94] = 'keypad 6',
    [95] = 'keypad 7',
    [96] = 'keypad 8',
    [97] = 'keypad 9',
    [98] = 'keypad 0',
    [99] = 'keypad .',
    [100] = 'key_100',
    [101] = 'menu',
    [102] = 'power',
    [103] = 'keypad =',
    [104] = 'f13',
    [105] = 'f14',
    [106] = 'f15',
    [107] = 'f16',
    [108] = 'f17',
    [109] = 'f18',
    [110] = 'f19',
    [111] = 'f20',
    [112] = 'f21',
    [113] = 'f22',
    [114] = 'f23',
    [115] = 'f24',
    [116] = 'execute',
    [117] = 'help',
    [118] = 'menu',
    [119] = 'select',
    [120] = 'stop',
    [121] = 'again',
    [122] = 'undo',
    [123] = 'cut',
    [124] = 'copy',
    [125] = 'paste',
    [126] = 'find',
    [127] = 'mute',
    [128] = 'volumeup',
    [129] = 'volumedown',
    [133] = 'keypad ,',
    [134] = 'keypad = (as400)',
    [135] = 'key_135',
    [136] = 'key_136',
    [137] = 'key_137',
    [138] = 'key_138',
    [139] = 'key_139',
    [140] = 'key_140',
    [141] = 'key_141',
    [142] = 'key_142',
    [143] = 'key_143',
    [144] = 'key_144',
    [145] = 'key_145',
    [146] = 'key_146',
    [147] = 'key_147',
    [148] = 'key_148',
    [149] = 'key_149',
    [150] = 'key_150',
    [151] = 'key_151',
    [152] = 'key_152',
    [153] = 'alterase',
    [154] = 'sysreq',
    [155] = 'cancel',
    [156] = 'clear',
    [157] = 'prior',
    [158] = 'return_2',
    [159] = 'separator',
    [160] = 'out',
    [161] = 'oper',
    [162] = 'clear / again',
    [163] = 'crsel',
    [164] = 'exsel',
    [176] = 'keypad 00',
    [177] = 'keypad 000',
    [178] = 'thousandsseparator',
    [179] = 'decimalseparator',
    [180] = 'currencyunit',
    [181] = 'currencysubunit',
    [182] = 'keypad (',
    [183] = 'keypad )',
    [184] = 'keypad {',
    [185] = 'keypad }',
    [186] = 'keypad tab',
    [187] = 'keypad backspace',
    [188] = 'keypad a',
    [189] = 'keypad b',
    [190] = 'keypad c',
    [191] = 'keypad d',
    [192] = 'keypad e',
    [193] = 'keypad f',
    [194] = 'keypad xor',
    [195] = 'keypad ^',
    [196] = 'keypad %',
    [197] = 'keypad <',
    [198] = 'keypad >',
    [199] = 'keypad &',
    [200] = 'keypad &&',
    [201] = 'keypad |',
    [202] = 'keypad ||',
    [203] = 'keypad :',
    [204] = 'keypad #',
    [205] = 'keypad space',
    [206] = 'keypad @',
    [207] = 'keypad !',
    [208] = 'keypad memstore',
    [209] = 'keypad memrecall',
    [210] = 'keypad memclear',
    [211] = 'keypad memadd',
    [212] = 'keypad memsubtract',
    [213] = 'keypad memmultiply',
    [214] = 'keypad memdivide',
    [215] = 'keypad +/-',
    [216] = 'keypad clear',
    [217] = 'keypad clearentry',
    [218] = 'keypad binary',
    [219] = 'keypad octal',
    [220] = 'keypad decimal',
    [221] = 'keypad hexadecimal',
    [224] = 'left ctrl',
    [225] = 'left shift',
    [226] = 'left alt',
    [227] = 'left windows',
    [228] = 'right ctrl',
    [229] = 'right shift',
    [230] = 'right alt',
    [231] = 'right windows',
    [257] = 'modeswitch',
    [258] = 'audionext',
    [259] = 'audioprev',
    [260] = 'audiostop',
    [261] = 'audioplay',
    [262] = 'audiomute',
    [263] = 'mediaselect',
    [264] = 'www',
    [265] = 'mail',
    [266] = 'calculator',
    [267] = 'computer',
    [268] = 'ac search',
    [269] = 'ac home',
    [270] = 'ac back',
    [271] = 'ac forward',
    [272] = 'ac stop',
    [273] = 'ac refresh',
    [274] = 'ac bookmarks',
    [275] = 'brightnessdown',
    [276] = 'brightnessup',
    [277] = 'displayswitch',
    [278] = 'kbdillumtoggle',
    [279] = 'kbdillumdown',
    [280] = 'kbdillumup',
    [281] = 'eject',
    [282] = 'sleep',
    [283] = 'key_283',
    [284] = 'key_284'
}
Keyboard._sc = fromScancode

local toScancode = {}
for i, v in pairs(fromScancode) do
    toScancode[v] = i
end

-- Key syms table
local fromKeysym = {
    [0] = 'key_unknown',
    [13] = 'return',
    [27] = 'escape',
    [8] = 'backspace',
    [9] = 'tab',
    [32] = 'space',
    [33] = '!',
    [34] = '"',
    [35] = '#',
    [37] = '%',
    [36] = '$',
    [38] = '&',
    [39] = '\'',
    [40] = '(',
    [41] = ')',
    [42] = '*',
    [43] = '+',
    [44] = ',',
    [45] = '-',
    [46] = '.',
    [47] = '/',
    [48] = '0',
    [49] = '1',
    [50] = '2',
    [51] = '3',
    [52] = '4',
    [53] = '5',
    [54] = '6',
    [55] = '7',
    [56] = '8',
    [57] = '9',
    [58] = ':',
    [59] = ';',
    [60] = '<',
    [61] = '=',
    [62] = '>',
    [63] = '?',
    [64] = '@',
    [91] = '[',
    [92] = '\\',
    [93] = ']',
    [94] = '^',
    [95] = '_',
    [96] = '`',
    [97] = 'a',
    [98] = 'b',
    [99] = 'c',
    [100] = 'd',
    [101] = 'e',
    [102] = 'f',
    [103] = 'g',
    [104] = 'h',
    [105] = 'i',
    [106] = 'j',
    [107] = 'k',
    [108] = 'l',
    [109] = 'm',
    [110] = 'n',
    [111] = 'o',
    [112] = 'p',
    [113] = 'q',
    [114] = 'r',
    [115] = 's',
    [116] = 't',
    [117] = 'u',
    [118] = 'v',
    [119] = 'w',
    [120] = 'x',
    [121] = 'y',
    [122] = 'z',
    [1073741881] = 'capslock',
    [1073741882] = 'f1',
    [1073741883] = 'f2',
    [1073741884] = 'f3',
    [1073741885] = 'f4',
    [1073741886] = 'f5',
    [1073741887] = 'f6',
    [1073741888] = 'f7',
    [1073741889] = 'f8',
    [1073741890] = 'f9',
    [1073741891] = 'f10',
    [1073741892] = 'f11',
    [1073741893] = 'f12',
    [1073741894] = 'printscreen',
    [1073741895] = 'scrolllock',
    [1073741896] = 'pause',
    [1073741897] = 'insert',
    [1073741898] = 'home',
    [1073741899] = 'pageup',
    [127] = 'delete',
    [1073741901] = 'end',
    [1073741902] = 'pagedown',
    [1073741903] = 'right',
    [1073741904] = 'left',
    [1073741905] = 'down',
    [1073741906] = 'up',
    [1073741907] = 'numlock',
    [1073741908] = 'keypad /',
    [1073741909] = 'keypad *',
    [1073741910] = 'keypad -',
    [1073741911] = 'keypad +',
    [1073741912] = 'keypad enter',
    [1073741913] = 'keypad 1',
    [1073741914] = 'keypad 2',
    [1073741915] = 'keypad 3',
    [1073741916] = 'keypad 4',
    [1073741917] = 'keypad 5',
    [1073741918] = 'keypad 6',
    [1073741919] = 'keypad 7',
    [1073741920] = 'keypad 8',
    [1073741921] = 'keypad 9',
    [1073741922] = 'keypad 0',
    [1073741923] = 'keypad .',
    [1073741925] = 'menu',
    [1073741926] = 'power',
    [1073741927] = 'keypad =',
    [1073741928] = 'f13',
    [1073741929] = 'f14',
    [1073741930] = 'f15',
    [1073741931] = 'f16',
    [1073741932] = 'f17',
    [1073741933] = 'f18',
    [1073741934] = 'f19',
    [1073741935] = 'f20',
    [1073741936] = 'f21',
    [1073741937] = 'f22',
    [1073741938] = 'f23',
    [1073741939] = 'f24',
    [1073741940] = 'execute',
    [1073741941] = 'help',
    [1073741942] = 'menu',
    [1073741943] = 'select',
    [1073741944] = 'stop',
    [1073741945] = 'again',
    [1073741946] = 'undo',
    [1073741947] = 'cut',
    [1073741948] = 'copy',
    [1073741949] = 'paste',
    [1073741950] = 'find',
    [1073741951] = 'mute',
    [1073741952] = 'volumeup',
    [1073741953] = 'volumedown',
    [1073741957] = 'keypad ,',
    [1073741958] = 'keypad = (as400)',
    [1073741977] = 'alterase',
    [1073741978] = 'sysreq',
    [1073741979] = 'cancel',
    [1073741980] = 'clear',
    [1073741981] = 'prior',
    [1073741982] = 'return',
    [1073741983] = 'separator',
    [1073741984] = 'out',
    [1073741985] = 'oper',
    [1073741986] = 'clear / again',
    [1073741987] = 'crsel',
    [1073741988] = 'exsel',
    [1073742000] = 'keypad 00',
    [1073742001] = 'keypad 000',
    [1073742002] = 'thousandsseparator',
    [1073742003] = 'decimalseparator',
    [1073742004] = 'currencyunit',
    [1073742005] = 'currencysubunit',
    [1073742006] = 'keypad (',
    [1073742007] = 'keypad )',
    [1073742008] = 'keypad {',
    [1073742009] = 'keypad }',
    [1073742010] = 'keypad tab',
    [1073742011] = 'keypad backspace',
    [1073742012] = 'keypad a',
    [1073742013] = 'keypad b',
    [1073742014] = 'keypad c',
    [1073742015] = 'keypad d',
    [1073742016] = 'keypad e',
    [1073742017] = 'keypad f',
    [1073742018] = 'keypad xor',
    [1073742019] = 'keypad ^',
    [1073742020] = 'keypad %',
    [1073742021] = 'keypad <',
    [1073742022] = 'keypad >',
    [1073742023] = 'keypad &',
    [1073742024] = 'keypad &&',
    [1073742025] = 'keypad |',
    [1073742026] = 'keypad ||',
    [1073742027] = 'keypad :',
    [1073742028] = 'keypad #',
    [1073742029] = 'keypad space',
    [1073742030] = 'keypad @',
    [1073742031] = 'keypad !',
    [1073742032] = 'keypad memstore',
    [1073742033] = 'keypad memrecall',
    [1073742034] = 'keypad memclear',
    [1073742035] = 'keypad memadd',
    [1073742036] = 'keypad memsubtract',
    [1073742037] = 'keypad memmultiply',
    [1073742038] = 'keypad memdivide',
    [1073742039] = 'keypad +/-',
    [1073742040] = 'keypad clear',
    [1073742041] = 'keypad clearentry',
    [1073742042] = 'keypad binary',
    [1073742043] = 'keypad octal',
    [1073742044] = 'keypad decimal',
    [1073742045] = 'keypad hexadecimal',
    [1073742048] = 'left ctrl',
    [1073742049] = 'left shift',
    [1073742050] = 'left alt',
    [1073742051] = 'left windows',
    [1073742052] = 'right ctrl',
    [1073742053] = 'right shift',
    [1073742054] = 'right alt',
    [1073742055] = 'right windows',
    [1073742081] = 'modeswitch',
    [1073742082] = 'audionext',
    [1073742083] = 'audioprev',
    [1073742084] = 'audiostop',
    [1073742085] = 'audioplay',
    [1073742086] = 'audiomute',
    [1073742087] = 'mediaselect',
    [1073742088] = 'www',
    [1073742089] = 'mail',
    [1073742090] = 'calculator',
    [1073742091] = 'computer',
    [1073742092] = 'ac search',
    [1073742093] = 'ac home',
    [1073742094] = 'ac back',
    [1073742095] = 'ac forward',
    [1073742096] = 'ac stop',
    [1073742097] = 'ac refresh',
    [1073742098] = 'ac bookmarks',
    [1073742099] = 'brightnessdown',
    [1073742100] = 'brightnessup',
    [1073742101] = 'displayswitch',
    [1073742102] = 'kbdillumtoggle',
    [1073742103] = 'kbdillumdown',
    [1073742104] = 'kbdillumup',
    [1073742105] = 'eject',
    [1073742106] = 'sleep'
}
Keyboard._sym = fromKeysym

local toKeysym = {}
for i, v in pairs(fromKeysym) do
    toKeysym[v] = i
end

-- Modifier keys table
local toModkey = {
    lshift = 1,
    rshift = 2,
    lctrl  = 64,
    rctrl  = 128,
    lalt   = 256,
    ralt   = 512,
    lmeta  = 1024,
    rmeta  = 2048,
    num    = 4096,
    caps   = 8192,
    altgr  = 16384,
    shift  = 3, -- lshift | rshift
    ctrl   = 192, -- lctrl | rctrl
    alt    = 768, -- lalt | ralt
    meta   = 3072 -- lmeta | rmeta
}
Keyboard._mod = toModkey

local fromModkey = {}
for i, v in pairs(toModkey) do
    fromModkey[v] = i
end

local downKeysSc = {}
local downKeysSym = {}

function Keyboard.KeyToScancode(key)
    return fromScancode[C.nxKeyboardToScancode(toKeysym[key] or 0)] or 'key_unknown'
end

function Keyboard.ScancodeToKey(key)
    return fromKeysym[C.nxKeyboardToKeysym(toScancode[key] or 0)] or 'key_unknown'
end

function Keyboard.modKeyDown(mod)
    mod = toModkey[mod]
    if not mod then return false end

    return C.nxKeyboardModKeyDown(mod)
end

function Keyboard.keysymDown(key)
    return downKeysSym[toKeysym[key] or 0]
end

function Keyboard.scancodeDown(key)
    return downKeysSc[toScancode[key] or 0]
end

function Keyboard.startTextInput(x, y, width, height)
    C.nxKeyboardStartTextInput(x, y, width, height)

    return Keyboard
end

function Keyboard.stopTextInput()
    C.nxKeyboardStopTextInput()

    return Keyboard
end

function Keyboard.isTextInputActive()
    return C.nxKeyboardTextInputActive()
end

function Keyboard.__keyDownEvent(scancode, keysym, isRepeated)
    if not isRepeated then
        downKeysSc[scancode] = true
        downKeysSym[keysym]  = true
    end
end

function Keyboard.__keyUpEvent(scancode, keysym)
    downKeysSc[scancode] = nil
    downKeysSym[keysym]  = nil
end

return Keyboard