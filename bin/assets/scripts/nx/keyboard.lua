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

--------------------------------------------------------------------------------

local Keyboard = {}

-- Scancodes table
local scancode = {
    [0] = 'KEY_UNKNOWN',
    [4] = 'A',
    [5] = 'B',
    [6] = 'C',
    [7] = 'D',
    [8] = 'E',
    [9] = 'F',
    [10] = 'G',
    [11] = 'H',
    [12] = 'I',
    [13] = 'J',
    [14] = 'K',
    [15] = 'L',
    [16] = 'M',
    [17] = 'N',
    [18] = 'O',
    [19] = 'P',
    [20] = 'Q',
    [21] = 'R',
    [22] = 'S',
    [23] = 'T',
    [24] = 'U',
    [25] = 'V',
    [26] = 'W',
    [27] = 'X',
    [28] = 'Y',
    [29] = 'Z',
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
    [40] = 'Return',
    [41] = 'Escape',
    [42] = 'Backspace',
    [43] = 'Tab',
    [44] = 'Space',
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
    [57] = 'CapsLock',
    [58] = 'F1',
    [59] = 'F2',
    [60] = 'F3',
    [61] = 'F4',
    [62] = 'F5',
    [63] = 'F6',
    [64] = 'F7',
    [65] = 'F8',
    [66] = 'F9',
    [67] = 'F10',
    [68] = 'F11',
    [69] = 'F12',
    [70] = 'PrintScreen',
    [71] = 'ScrollLock',
    [72] = 'Pause',
    [73] = 'Insert',
    [74] = 'Home',
    [75] = 'PageUp',
    [76] = 'Delete',
    [77] = 'End',
    [78] = 'PageDown',
    [79] = 'Right',
    [80] = 'Left',
    [81] = 'Down',
    [82] = 'Up',
    [83] = 'Numlock',
    [84] = 'Keypad /',
    [85] = 'Keypad *',
    [86] = 'Keypad -',
    [87] = 'Keypad +',
    [88] = 'Keypad Enter',
    [89] = 'Keypad 1',
    [90] = 'Keypad 2',
    [91] = 'Keypad 3',
    [92] = 'Keypad 4',
    [93] = 'Keypad 5',
    [94] = 'Keypad 6',
    [95] = 'Keypad 7',
    [96] = 'Keypad 8',
    [97] = 'Keypad 9',
    [98] = 'Keypad 0',
    [99] = 'Keypad .',
    [100] = 'KEY_100',
    [101] = 'Menu',
    [102] = 'Power',
    [103] = 'Keypad =',
    [104] = 'F13',
    [105] = 'F14',
    [106] = 'F15',
    [107] = 'F16',
    [108] = 'F17',
    [109] = 'F18',
    [110] = 'F19',
    [111] = 'F20',
    [112] = 'F21',
    [113] = 'F22',
    [114] = 'F23',
    [115] = 'F24',
    [116] = 'Execute',
    [117] = 'Help',
    [118] = 'Menu',
    [119] = 'Select',
    [120] = 'Stop',
    [121] = 'Again',
    [122] = 'Undo',
    [123] = 'Cut',
    [124] = 'Copy',
    [125] = 'Paste',
    [126] = 'Find',
    [127] = 'Mute',
    [128] = 'VolumeUp',
    [129] = 'VolumeDown',
    [133] = 'Keypad ,',
    [134] = 'Keypad = (AS400)',
    [135] = 'KEY_135',
    [136] = 'KEY_136',
    [137] = 'KEY_137',
    [138] = 'KEY_138',
    [139] = 'KEY_139',
    [140] = 'KEY_140',
    [141] = 'KEY_141',
    [142] = 'KEY_142',
    [143] = 'KEY_143',
    [144] = 'KEY_144',
    [145] = 'KEY_145',
    [146] = 'KEY_146',
    [147] = 'KEY_147',
    [148] = 'KEY_148',
    [149] = 'KEY_149',
    [150] = 'KEY_150',
    [151] = 'KEY_151',
    [152] = 'KEY_152',
    [153] = 'AltErase',
    [154] = 'SysReq',
    [155] = 'Cancel',
    [156] = 'Clear',
    [157] = 'Prior',
    [158] = 'Return',
    [159] = 'Separator',
    [160] = 'Out',
    [161] = 'Oper',
    [162] = 'Clear / Again',
    [163] = 'CrSel',
    [164] = 'ExSel',
    [176] = 'Keypad 00',
    [177] = 'Keypad 000',
    [178] = 'ThousandsSeparator',
    [179] = 'DecimalSeparator',
    [180] = 'CurrencyUnit',
    [181] = 'CurrencySubUnit',
    [182] = 'Keypad (',
    [183] = 'Keypad )',
    [184] = 'Keypad {',
    [185] = 'Keypad }',
    [186] = 'Keypad Tab',
    [187] = 'Keypad Backspace',
    [188] = 'Keypad A',
    [189] = 'Keypad B',
    [190] = 'Keypad C',
    [191] = 'Keypad D',
    [192] = 'Keypad E',
    [193] = 'Keypad F',
    [194] = 'Keypad XOR',
    [195] = 'Keypad ^',
    [196] = 'Keypad %',
    [197] = 'Keypad <',
    [198] = 'Keypad >',
    [199] = 'Keypad &',
    [200] = 'Keypad &&',
    [201] = 'Keypad |',
    [202] = 'Keypad ||',
    [203] = 'Keypad :',
    [204] = 'Keypad #',
    [205] = 'Keypad Space',
    [206] = 'Keypad @',
    [207] = 'Keypad !',
    [208] = 'Keypad MemStore',
    [209] = 'Keypad MemRecall',
    [210] = 'Keypad MemClear',
    [211] = 'Keypad MemAdd',
    [212] = 'Keypad MemSubtract',
    [213] = 'Keypad MemMultiply',
    [214] = 'Keypad MemDivide',
    [215] = 'Keypad +/-',
    [216] = 'Keypad Clear',
    [217] = 'Keypad ClearEntry',
    [218] = 'Keypad Binary',
    [219] = 'Keypad Octal',
    [220] = 'Keypad Decimal',
    [221] = 'Keypad Hexadecimal',
    [224] = 'Left Ctrl',
    [225] = 'Left Shift',
    [226] = 'Left Alt',
    [227] = 'Left Windows',
    [228] = 'Right Ctrl',
    [229] = 'Right Shift',
    [230] = 'Right Alt',
    [231] = 'Right Windows',
    [257] = 'ModeSwitch',
    [258] = 'AudioNext',
    [259] = 'AudioPrev',
    [260] = 'AudioStop',
    [261] = 'AudioPlay',
    [262] = 'AudioMute',
    [263] = 'MediaSelect',
    [264] = 'WWW',
    [265] = 'Mail',
    [266] = 'Calculator',
    [267] = 'Computer',
    [268] = 'AC Search',
    [269] = 'AC Home',
    [270] = 'AC Back',
    [271] = 'AC Forward',
    [272] = 'AC Stop',
    [273] = 'AC Refresh',
    [274] = 'AC Bookmarks',
    [275] = 'BrightnessDown',
    [276] = 'BrightnessUp',
    [277] = 'DisplaySwitch',
    [278] = 'KBDIllumToggle',
    [279] = 'KBDIllumDown',
    [280] = 'KBDIllumUp',
    [281] = 'Eject',
    [282] = 'Sleep',
    [283] = 'KEY_283',
    [284] = 'KEY_284'
}
Keyboard._sc = scancode

-- Key syms table
local keysym = {
    [0] = 'Key_Unknown',
    [13] = 'Return',
    [27] = 'Escape',
    [8] = 'Backspace',
    [9] = 'Tab',
    [32] = 'Space',
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
    [97] = 'A',
    [98] = 'B',
    [99] = 'C',
    [100] = 'D',
    [101] = 'E',
    [102] = 'F',
    [103] = 'G',
    [104] = 'H',
    [105] = 'I',
    [106] = 'J',
    [107] = 'K',
    [108] = 'L',
    [109] = 'M',
    [110] = 'N',
    [111] = 'O',
    [112] = 'P',
    [113] = 'Q',
    [114] = 'R',
    [115] = 'S',
    [116] = 'T',
    [117] = 'U',
    [118] = 'V',
    [119] = 'W',
    [120] = 'X',
    [121] = 'Y',
    [122] = 'Z',
    [1073741881] = 'CapsLock',
    [1073741882] = 'F1',
    [1073741883] = 'F2',
    [1073741884] = 'F3',
    [1073741885] = 'F4',
    [1073741886] = 'F5',
    [1073741887] = 'F6',
    [1073741888] = 'F7',
    [1073741889] = 'F8',
    [1073741890] = 'F9',
    [1073741891] = 'F10',
    [1073741892] = 'F11',
    [1073741893] = 'F12',
    [1073741894] = 'PrintScreen',
    [1073741895] = 'ScrollLock',
    [1073741896] = 'Pause',
    [1073741897] = 'Insert',
    [1073741898] = 'Home',
    [1073741899] = 'PageUp',
    [127] = 'Delete',
    [1073741901] = 'End',
    [1073741902] = 'PageDown',
    [1073741903] = 'Right',
    [1073741904] = 'Left',
    [1073741905] = 'Down',
    [1073741906] = 'Up',
    [1073741907] = 'Numlock',
    [1073741908] = 'Keypad /',
    [1073741909] = 'Keypad *',
    [1073741910] = 'Keypad -',
    [1073741911] = 'Keypad +',
    [1073741912] = 'Keypad Enter',
    [1073741913] = 'Keypad 1',
    [1073741914] = 'Keypad 2',
    [1073741915] = 'Keypad 3',
    [1073741916] = 'Keypad 4',
    [1073741917] = 'Keypad 5',
    [1073741918] = 'Keypad 6',
    [1073741919] = 'Keypad 7',
    [1073741920] = 'Keypad 8',
    [1073741921] = 'Keypad 9',
    [1073741922] = 'Keypad 0',
    [1073741923] = 'Keypad .',
    [1073741925] = 'Menu',
    [1073741926] = 'Power',
    [1073741927] = 'Keypad =',
    [1073741928] = 'F13',
    [1073741929] = 'F14',
    [1073741930] = 'F15',
    [1073741931] = 'F16',
    [1073741932] = 'F17',
    [1073741933] = 'F18',
    [1073741934] = 'F19',
    [1073741935] = 'F20',
    [1073741936] = 'F21',
    [1073741937] = 'F22',
    [1073741938] = 'F23',
    [1073741939] = 'F24',
    [1073741940] = 'Execute',
    [1073741941] = 'Help',
    [1073741942] = 'Menu',
    [1073741943] = 'Select',
    [1073741944] = 'Stop',
    [1073741945] = 'Again',
    [1073741946] = 'Undo',
    [1073741947] = 'Cut',
    [1073741948] = 'Copy',
    [1073741949] = 'Paste',
    [1073741950] = 'Find',
    [1073741951] = 'Mute',
    [1073741952] = 'VolumeUp',
    [1073741953] = 'VolumeDown',
    [1073741957] = 'Keypad ,',
    [1073741958] = 'Keypad = (AS400)',
    [1073741977] = 'AltErase',
    [1073741978] = 'SysReq',
    [1073741979] = 'Cancel',
    [1073741980] = 'Clear',
    [1073741981] = 'Prior',
    [1073741982] = 'Return',
    [1073741983] = 'Separator',
    [1073741984] = 'Out',
    [1073741985] = 'Oper',
    [1073741986] = 'Clear / Again',
    [1073741987] = 'CrSel',
    [1073741988] = 'ExSel',
    [1073742000] = 'Keypad 00',
    [1073742001] = 'Keypad 000',
    [1073742002] = 'ThousandsSeparator',
    [1073742003] = 'DecimalSeparator',
    [1073742004] = 'CurrencyUnit',
    [1073742005] = 'CurrencySubUnit',
    [1073742006] = 'Keypad (',
    [1073742007] = 'Keypad )',
    [1073742008] = 'Keypad {',
    [1073742009] = 'Keypad }',
    [1073742010] = 'Keypad Tab',
    [1073742011] = 'Keypad Backspace',
    [1073742012] = 'Keypad A',
    [1073742013] = 'Keypad B',
    [1073742014] = 'Keypad C',
    [1073742015] = 'Keypad D',
    [1073742016] = 'Keypad E',
    [1073742017] = 'Keypad F',
    [1073742018] = 'Keypad XOR',
    [1073742019] = 'Keypad ^',
    [1073742020] = 'Keypad %',
    [1073742021] = 'Keypad <',
    [1073742022] = 'Keypad >',
    [1073742023] = 'Keypad &',
    [1073742024] = 'Keypad &&',
    [1073742025] = 'Keypad |',
    [1073742026] = 'Keypad ||',
    [1073742027] = 'Keypad :',
    [1073742028] = 'Keypad #',
    [1073742029] = 'Keypad Space',
    [1073742030] = 'Keypad @',
    [1073742031] = 'Keypad !',
    [1073742032] = 'Keypad MemStore',
    [1073742033] = 'Keypad MemRecall',
    [1073742034] = 'Keypad MemClear',
    [1073742035] = 'Keypad MemAdd',
    [1073742036] = 'Keypad MemSubtract',
    [1073742037] = 'Keypad MemMultiply',
    [1073742038] = 'Keypad MemDivide',
    [1073742039] = 'Keypad +/-',
    [1073742040] = 'Keypad Clear',
    [1073742041] = 'Keypad ClearEntry',
    [1073742042] = 'Keypad Binary',
    [1073742043] = 'Keypad Octal',
    [1073742044] = 'Keypad Decimal',
    [1073742045] = 'Keypad Hexadecimal',
    [1073742048] = 'Left Ctrl',
    [1073742049] = 'Left Shift',
    [1073742050] = 'Left Alt',
    [1073742051] = 'Left Windows',
    [1073742052] = 'Right Ctrl',
    [1073742053] = 'Right Shift',
    [1073742054] = 'Right Alt',
    [1073742055] = 'Right Windows',
    [1073742081] = 'ModeSwitch',
    [1073742082] = 'AudioNext',
    [1073742083] = 'AudioPrev',
    [1073742084] = 'AudioStop',
    [1073742085] = 'AudioPlay',
    [1073742086] = 'AudioMute',
    [1073742087] = 'MediaSelect',
    [1073742088] = 'WWW',
    [1073742089] = 'Mail',
    [1073742090] = 'Calculator',
    [1073742091] = 'Computer',
    [1073742092] = 'AC Search',
    [1073742093] = 'AC Home',
    [1073742094] = 'AC Back',
    [1073742095] = 'AC Forward',
    [1073742096] = 'AC Stop',
    [1073742097] = 'AC Refresh',
    [1073742098] = 'AC Bookmarks',
    [1073742099] = 'BrightnessDown',
    [1073742100] = 'BrightnessUp',
    [1073742101] = 'DisplaySwitch',
    [1073742102] = 'KBDIllumToggle',
    [1073742103] = 'KBDIllumDown',
    [1073742104] = 'KBDIllumUp',
    [1073742105] = 'Eject',
    [1073742106] = 'Sleep'
}
Keyboard._sym = keysym

-- Modifier keys table
local modkey = {
    lshift = 1,
    rshift = 2,
    lctrl = 64,
    rctrl = 128,
    lalt = 256,
    ralt = 512,
    lmeta = 1024,
    rmeta = 2048,
    num = 4096,
    caps = 8192,
    altgr = 16384,
    ctrl = 192,
    shift = 3,
    alt = 768,
    meta = 3072
}
Keyboard._mod = modkey

function Keyboard.KeyToScancode(key)
    for i = 0, #keysym do
        if keysym[i] == key then
            return C.nxKeyboardToScancode(keysym[i])
        end
    end
end

function Keyboard.ScancodeToKey(key)
    for i = 0, #scancode do
        if scancode[i] == key then
            return C.nxKeyboardToKeysym(scancode[i])
        end
    end
end

function Keyboard.modKeyDown(mod)
    local mod = modkey[mod]
    if not mod then return false end

    return C.nxKeyboardModKeyDown(mod)
end

function Keyboard.startTextInput(x, y, width, height)
    C.nxKeyboardStartTextInput(x, y, width, height)
end

function Keyboard.stopTextInput()
    C.nxKeyboardStopTextInput()
end

function Keyboard.isTextInputActive()
    return C.nxKeyboardTextInputActive() == true
end

return Keyboard