local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef enum {
        NX_NoEvent = 0,
        NX_Other,
        NX_Quit,
        NX_Resized,
        NX_Visible,
        NX_Focus,
        NX_LowMemory,
        NX_TextEntered,
        NX_TextEdited,
        NX_KeyPressed,
        NX_KeyReleased,
        NX_MouseMoved,
        NX_MouseFocus,
        NX_MouseButtonPressed,
        NX_MouseButtonReleased,
        NX_MouseWheelScrolled,
        NX_JoystickMoved,
        NX_JoystickButtonPressed,
        NX_JoystickButtonReleased,
        NX_JoystickConnect,
        NX_TouchBegan,
        NX_TouchEnded,
        NX_TouchMoved,
        NX_ClipboardUpdated,
        NX_FileDropped
    } NxEventType;

    typedef struct {
        double a, b, c;
        const char* t;
    } NxEvent;

    NxEventType nxEventPoll(NxEvent*);
]]

local Events = {}

local mouseButtons = {
    'left',
    'middle',
    'right',
    'xbutton1',
    'xbutton2'
}

local scanCodes = {
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

local function fromSym(sym)
    return tonumber(sym)
end

--------------------------------------------------------------------------------
function Events.poll()
    local function pollFunc(t, i)
        local evType
        local evPtr = ffi.new('NxEvent[1]')
        repeat
            evType = C.nxEventPoll(evPtr)
        until evType ~= NX_Other
        e = evPtr[0]

        if evType == C.NX_NoEvent then
            return nil
        elseif evType == C.NX_Quit then
            return 'quit'
        elseif evType == C.NX_Resized then
            return 'resized', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_Visible then
            return 'visible', (e.a == 1.0)
        elseif evType == C.NX_Focus then
            return 'focus', (e.a == 1.0)
        elseif evType == C.NX_MouseFocus then
            return 'mousefocus', (e.a == 1.0)
        elseif evType == C.NX_TextEntered then
            return 'textinput', ffi.string(e.t)
        elseif evType == C.NX_TextEdited then
            return 'textedit', ffi.string(e.t), tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_KeyPressed then
            return 'keydown', e.a ~= 0, scanCodes[e.b], fromSym(e.c)
        elseif evType == C.NX_KeyReleased then
            return 'keyup', e.a ~= 0, scanCodes[e.b], fromSym(e.c)
        elseif evType == C.NX_MouseMoved then
            return 'mousemotion', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_MouseButtonPressed then
            return 'mousedown', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
        elseif evType == C.NX_MouseButtonReleased then
            return 'mouseup', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
        elseif evType == C.NX_MouseWheelScrolled then
            return 'wheelscrolled', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickMoved then
            return 'joymotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_JoystickButtonPressed then
            return 'joybuttondown', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickButtonReleased then
            return 'joybuttonup', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickConnect then
            return 'joyconnect', tonumber(e.a), (e.b == 1.0)
        elseif evType == C.NX_TouchBegan then
            return 'touchdown', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_TouchEnded then
            return 'touchup', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_TouchMoved then
            return 'touchmotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_FileDropped then
            return 'filedropped', ffi.string(e.t)
        else
            return 'other'
        end
    end
    return pollFunc, nil, nil
end

return Events