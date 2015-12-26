--[[----------------------------------------------------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
--]]----------------------------------------------------------------------------

local OriginalAlfUpperMdd   = 0x0622
local OriginalAlfUpperHamza = 0x0623
local OriginalAlfLowerHamza = 0x0625
local OriginalAlf           = 0x0627
local OriginalLam           = 0x0644

local LamAlefGlyphs = {
    { 0x0622, 0xFEF6, 0xFEF5 },
    { 0x0623, 0xFEF8, 0xFEF7 },
    { 0x0627, 0xFEFC, 0xFEFB },
    { 0x0625, 0xFEFA, 0xFEF9 }
}

local ShaddaHarakatGlyphs = {
    [ 0x064C ] = 0xFC5E, -- Dammatan + Shadda
    [ 0x064D ] = 0xFC5F, -- Kasratan + Shadda
    [ 0x064E ] = 0xFC60, -- Fatha + Shadda
    [ 0x065F ] = 0xFC61, -- Damma + Shadda
    [ 0x0650 ] = 0xFC62, -- Kasra + Shadda
    [ 0x0670 ] = 0xFC63, -- Superscript Alef + Shadda
}

local Harakat = {
    [ 0x0600 ] = true, [ 0x0601 ] = true, [ 0x0602 ] = true, [ 0x0603 ] = true,
    [ 0x0606 ] = true, [ 0x0607 ] = true, [ 0x0608 ] = true, [ 0x0609 ] = true,
    [ 0x060A ] = true, [ 0x060B ] = true, [ 0x060D ] = true, [ 0x060E ] = true,
    [ 0x0610 ] = true, [ 0x0611 ] = true, [ 0x0612 ] = true, [ 0x0613 ] = true,
    [ 0x0614 ] = true, [ 0x0615 ] = true, [ 0x0616 ] = true, [ 0x0617 ] = true,
    [ 0x0618 ] = true, [ 0x0619 ] = true, [ 0x061A ] = true, [ 0x061B ] = true,
    [ 0x061E ] = true, [ 0x061F ] = true, [ 0x0621 ] = true, [ 0x063B ] = true,
    [ 0x063C ] = true, [ 0x063D ] = true, [ 0x063E ] = true, [ 0x063F ] = true,
    [ 0x0640 ] = true, [ 0x064B ] = true, [ 0x064C ] = true, [ 0x064D ] = true,
    [ 0x064E ] = true, [ 0x064F ] = true, [ 0x0650 ] = true, [ 0x0651 ] = true,
    [ 0x0652 ] = true, [ 0x0653 ] = true, [ 0x0654 ] = true, [ 0x0655 ] = true,
    [ 0x0656 ] = true, [ 0x0657 ] = true, [ 0x0658 ] = true, [ 0x0659 ] = true,
    [ 0x065A ] = true, [ 0x065B ] = true, [ 0x065C ] = true, [ 0x065D ] = true,
    [ 0x065E ] = true, [ 0x0660 ] = true, [ 0x066A ] = true, [ 0x066B ] = true,
    [ 0x066C ] = true, [ 0x066F ] = true, [ 0x0670 ] = true, [ 0x0672 ] = true,
    [ 0x06D4 ] = true, [ 0x06D5 ] = true, [ 0x06D6 ] = true, [ 0x06D7 ] = true,
    [ 0x06D8 ] = true, [ 0x06D9 ] = true, [ 0x06DA ] = true, [ 0x06DB ] = true,
    [ 0x06DC ] = true, [ 0x06DF ] = true, [ 0x06E0 ] = true, [ 0x06E1 ] = true,
    [ 0x06E2 ] = true, [ 0x06E3 ] = true, [ 0x06E4 ] = true, [ 0x06E5 ] = true,
    [ 0x06E6 ] = true, [ 0x06E7 ] = true, [ 0x06E8 ] = true, [ 0x06E9 ] = true,
    [ 0x06EA ] = true, [ 0x06EB ] = true, [ 0x06EC ] = true, [ 0x06ED ] = true,
    [ 0x06EE ] = true, [ 0x06EF ] = true, [ 0x06D6 ] = true, [ 0x06D7 ] = true,
    [ 0x06D8 ] = true, [ 0x06D9 ] = true, [ 0x06DA ] = true, [ 0x06DB ] = true,
    [ 0x06DC ] = true, [ 0x06DD ] = true, [ 0x06DE ] = true, [ 0x06DF ] = true,
    [ 0x06F0 ] = true, [ 0x06FD ] = true, [ 0xFE70 ] = true, [ 0xFE71 ] = true,
    [ 0xFE72 ] = true, [ 0xFE73 ] = true, [ 0xFE74 ] = true, [ 0xFE75 ] = true,
    [ 0xFE76 ] = true, [ 0xFE77 ] = true, [ 0xFE78 ] = true, [ 0xFE79 ] = true,
    [ 0xFE7A ] = true, [ 0xFE7B ] = true, [ 0xFE7C ] = true, [ 0xFE7D ] = true,
    [ 0xFE7E ] = true, [ 0xFE7F ] = true, [ 0xFC5E ] = true, [ 0xFC5F ] = true,
    [ 0xFC60 ] = true, [ 0xFC61 ] = true, [ 0xFC62 ] = true, [ 0xFC63 ] = true
}

local Glyphs = {
    [ 0x0622 ] = { 0x0622, 0xFE81, 0xFE81, 0xFE82, 0xFE82, 2 },
    [ 0x0623 ] = { 0x0623, 0xFE83, 0xFE83, 0xFE84, 0xFE84, 2 },
    [ 0x0624 ] = { 0x0624, 0xFE85, 0xFE85, 0xFE86, 0xFE86, 2 },
    [ 0x0625 ] = { 0x0625, 0xFE87, 0xFE87, 0xFE88, 0xFE88, 2 },
    [ 0x0626 ] = { 0x0626, 0xFE89, 0xFE8B, 0xFE8C, 0xFE8A, 4 },
    [ 0x0627 ] = { 0x0627, 0x0627, 0x0627, 0xFE8E, 0xFE8E, 2 },
    [ 0x0628 ] = { 0x0628, 0xFE8F, 0xFE91, 0xFE92, 0xFE90, 4 },
    [ 0x0629 ] = { 0x0629, 0xFE93, 0xFE93, 0xFE94, 0xFE94, 2 },
    [ 0x062A ] = { 0x062A, 0xFE95, 0xFE97, 0xFE98, 0xFE96, 4 },
    [ 0x062B ] = { 0x062B, 0xFE99, 0xFE9B, 0xFE9C, 0xFE9A, 4 },
    [ 0x062C ] = { 0x062C, 0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E, 4 },
    [ 0x062D ] = { 0x062D, 0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2, 4 },
    [ 0x062E ] = { 0x062E, 0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6, 4 },
    [ 0x062F ] = { 0x062F, 0xFEA9, 0xFEA9, 0xFEAA, 0xFEAA, 2 },
    [ 0x0630 ] = { 0x0630, 0xFEAB, 0xFEAB, 0xFEAC, 0xFEAC, 2 },
    [ 0x0631 ] = { 0x0631, 0xFEAD, 0xFEAD, 0xFEAE, 0xFEAE, 2 },
    [ 0x0632 ] = { 0x0632, 0xFEAF, 0xFEAF, 0xFEB0, 0xFEB0, 2 },
    [ 0x0633 ] = { 0x0633, 0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2, 4 },
    [ 0x0634 ] = { 0x0634, 0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6, 4 },
    [ 0x0635 ] = { 0x0635, 0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA, 4 },
    [ 0x0636 ] = { 0x0636, 0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE, 4 },
    [ 0x0637 ] = { 0x0637, 0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2, 4 },
    [ 0x0638 ] = { 0x0638, 0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6, 4 },
    [ 0x0639 ] = { 0x0639, 0xFEC9, 0xFECB, 0xFECC, 0xFECA, 4 },
    [ 0x063A ] = { 0x063A, 0xFECD, 0xFECF, 0xFED0, 0xFECE, 4 },
    [ 0x0641 ] = { 0x0641, 0xFED1, 0xFED3, 0xFED4, 0xFED2, 4 },
    [ 0x0642 ] = { 0x0642, 0xFED5, 0xFED7, 0xFED8, 0xFED6, 4 },
    [ 0x0643 ] = { 0x0643, 0xFED9, 0xFEDB, 0xFEDC, 0xFEDA, 4 },
    [ 0x0644 ] = { 0x0644, 0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE, 4 },
    [ 0x0645 ] = { 0x0645, 0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2, 4 },
    [ 0x0646 ] = { 0x0646, 0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6, 4 },
    [ 0x0647 ] = { 0x0647, 0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA, 4 },
    [ 0x0648 ] = { 0x0648, 0xFEED, 0xFEED, 0xFEEE, 0xFEEE, 2 },
    [ 0x0649 ] = { 0x0649, 0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0, 2 },
    [ 0x0671 ] = { 0x0671, 0x0671, 0x0671, 0xFB51, 0xFB51, 2 },
    [ 0x064A ] = { 0x064A, 0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2, 4 },
    [ 0x066E ] = { 0x066E, 0xFBE4, 0xFBE8, 0xFBE9, 0xFBE5, 4 },
    [ 0x06AA ] = { 0x06AA, 0xFB8E, 0xFB90, 0xFB91, 0xFB8F, 4 },
    [ 0x06C1 ] = { 0x06C1, 0xFBA6, 0xFBA8, 0xFBA9, 0xFBA7, 4 },
    [ 0x06E4 ] = { 0x06E4, 0x06E4, 0x06E4, 0x06E4, 0xFEEE, 2 },
    [ 0x067E ] = { 0x067E, 0xFB56, 0xFB58, 0xFB59, 0xFB57, 4 },
    [ 0x0698 ] = { 0x0698, 0xFB8A, 0xFB8A, 0xFB8A, 0xFB8B, 2 },
    [ 0x06AF ] = { 0x06AF, 0xFB92, 0xFB94, 0xFB95, 0xFB93, 4 },
    [ 0x0686 ] = { 0x0686, 0xFB7A, 0xFB7C, 0xFB7D, 0xFB7B, 4 },
    [ 0x06A9 ] = { 0x06A9, 0xFB8E, 0xFB90, 0xFB91, 0xFB8F, 4 },
    [ 0x06CC ] = { 0x06CC, 0xFEEF, 0xFEF3, 0xFEF4, 0xFEF0, 4 }
}

local GlyphList = {
    { 0x0622, 0xFE81, 0xFE81, 0xFE82, 0xFE82, 2 },
    { 0x0623, 0xFE83, 0xFE83, 0xFE84, 0xFE84, 2 },
    { 0x0624, 0xFE85, 0xFE85, 0xFE86, 0xFE86, 2 },
    { 0x0625, 0xFE87, 0xFE87, 0xFE88, 0xFE88, 2 },
    { 0x0626, 0xFE89, 0xFE8B, 0xFE8C, 0xFE8A, 4 },
    { 0x0627, 0x0627, 0x0627, 0xFE8E, 0xFE8E, 2 },
    { 0x0628, 0xFE8F, 0xFE91, 0xFE92, 0xFE90, 4 },
    { 0x0629, 0xFE93, 0xFE93, 0xFE94, 0xFE94, 2 },
    { 0x062A, 0xFE95, 0xFE97, 0xFE98, 0xFE96, 4 },
    { 0x062B, 0xFE99, 0xFE9B, 0xFE9C, 0xFE9A, 4 },
    { 0x062C, 0xFE9D, 0xFE9F, 0xFEA0, 0xFE9E, 4 },
    { 0x062D, 0xFEA1, 0xFEA3, 0xFEA4, 0xFEA2, 4 },
    { 0x062E, 0xFEA5, 0xFEA7, 0xFEA8, 0xFEA6, 4 },
    { 0x062F, 0xFEA9, 0xFEA9, 0xFEAA, 0xFEAA, 2 },
    { 0x0630, 0xFEAB, 0xFEAB, 0xFEAC, 0xFEAC, 2 },
    { 0x0631, 0xFEAD, 0xFEAD, 0xFEAE, 0xFEAE, 2 },
    { 0x0632, 0xFEAF, 0xFEAF, 0xFEB0, 0xFEB0, 2 },
    { 0x0633, 0xFEB1, 0xFEB3, 0xFEB4, 0xFEB2, 4 },
    { 0x0634, 0xFEB5, 0xFEB7, 0xFEB8, 0xFEB6, 4 },
    { 0x0635, 0xFEB9, 0xFEBB, 0xFEBC, 0xFEBA, 4 },
    { 0x0636, 0xFEBD, 0xFEBF, 0xFEC0, 0xFEBE, 4 },
    { 0x0637, 0xFEC1, 0xFEC3, 0xFEC4, 0xFEC2, 4 },
    { 0x0638, 0xFEC5, 0xFEC7, 0xFEC8, 0xFEC6, 4 },
    { 0x0639, 0xFEC9, 0xFECB, 0xFECC, 0xFECA, 4 },
    { 0x063A, 0xFECD, 0xFECF, 0xFED0, 0xFECE, 4 },
    { 0x0641, 0xFED1, 0xFED3, 0xFED4, 0xFED2, 4 },
    { 0x0642, 0xFED5, 0xFED7, 0xFED8, 0xFED6, 4 },
    { 0x0643, 0xFED9, 0xFEDB, 0xFEDC, 0xFEDA, 4 },
    { 0x0644, 0xFEDD, 0xFEDF, 0xFEE0, 0xFEDE, 4 },
    { 0x0645, 0xFEE1, 0xFEE3, 0xFEE4, 0xFEE2, 4 },
    { 0x0646, 0xFEE5, 0xFEE7, 0xFEE8, 0xFEE6, 4 },
    { 0x0647, 0xFEE9, 0xFEEB, 0xFEEC, 0xFEEA, 4 },
    { 0x0648, 0xFEED, 0xFEED, 0xFEEE, 0xFEEE, 2 },
    { 0x0649, 0xFEEF, 0xFEEF, 0xFEF0, 0xFEF0, 2 },
    { 0x0671, 0x0671, 0x0671, 0xFB51, 0xFB51, 2 },
    { 0x064A, 0xFEF1, 0xFEF3, 0xFEF4, 0xFEF2, 4 },
    { 0x066E, 0xFBE4, 0xFBE8, 0xFBE9, 0xFBE5, 4 },
    { 0x06AA, 0xFB8E, 0xFB90, 0xFB91, 0xFB8F, 4 },
    { 0x06C1, 0xFBA6, 0xFBA8, 0xFBA9, 0xFBA7, 4 },
    { 0x067E, 0xFB56, 0xFB58, 0xFB59, 0xFB57, 4 },
    { 0x0698, 0xFB8A, 0xFB8A, 0xFB8A, 0xFB8B, 2 },
    { 0x06AF, 0xFB92, 0xFB94, 0xFB95, 0xFB93, 4 },
    { 0x0686, 0xFB7A, 0xFB7C, 0xFB7D, 0xFB7B, 4 },
    { 0x06A9, 0xFB8E, 0xFB90, 0xFB91, 0xFB8F, 4 },
    { 0x06CC, 0xFEEF, 0xFEF3, 0xFEF4, 0xFEF0, 4 }
}

local function getReshapedGlyph(target, location)
    local glyph = Glyphs[target]
    if glyph then return glyph[location] end
    
    return target
end

local function getGlyphType(target)
    local glyph = Glyphs[target]
    if glyph then return glyph[6] end

    return 2
end

local function getLamAlef(candidateAlef, candidateLam, isEndOfWord)
    local shift = isEndOfWord and 3 or 2
    local reshapedAlefLam

    if candidateLam == OriginalLam then
        if candidateAlef == OriginalAlfUpperMdd then
            reshapedAlefLam = LamAlefGlyphs[1][shift]
        elseif candidateAlef == OriginalAlfUpperHamza then
            reshapedAlefLam = LamAlefGlyphs[2][shift]
        elseif candidateAlef == OriginalAlf then
            reshapedAlefLam = LamAlefGlyphs[3][shift]
        elseif candidateAlef == OriginalAlfLowerHamza then
            reshapedAlefLam = LamAlefGlyphs[4][shift]
        end
    end

    return reshapedAlefLam
end

local function replaceShadda(unshapedWord)
    local word = {}
    local skipNext = false

    for i, c in ipairs(unshapedWord) do
        if skipNext then
            skipNext = false
        elseif c == 0x651 then
            if ShaddaHarakatGlyphs[unshapedWord[i-1]] then
                word[#word] = ShaddaHarakatGlyphs[unshapedWord[i-1]]
            elseif ShaddaHarakatGlyphs[unshapedWord[i+1]] then
                word[#word+1] = ShaddaHarakatGlyphs[unshapedWord[i+1]]
                skipNext = true
            else
                word[#word+1] = c
            end
        else
            word[#word+1] = c
        end
    end

    return word
end

local function replaceLamAlef(unshapedWord)
    -- Copy letters
    local letters = {}
    for i, v in ipairs(unshapedWord) do
        letters[i] = v
    end

    local currLetter, prevLetter = 0, 0
    for i = 1, #letters - 1 do
        currLetter = letters[i]
        if not Harakat[currLetter] and currLetter ~= OriginalLam then
            prevLetter = currLetter
        end
        if currLetter == OriginalLam then
            local candidateLam = currLetter
            local lamPosition = i
            local harakatPosition = lamPosition + 1

            while harakatPosition < #letters and Harakat[letters[harakatPosition]] do
                harakatPosition = harakatPosition + 1
            end

            if harakatPosition < #letters then
                local lamAlef
                if getGlyphType(prevLetter) > 2 then
                    lamAlef = getLamAlef(letters[harakatPosition], candidateLam, false)
                else
                    lamAlef = getLamAlef(letters[harakatPosition], candidateLam, true)
                end
                if lamAlef then
                    letters[lamPosition] = lamAlef
                    letters[harakatPosition] = false
                end
            end
        end
    end

    unshapedWord = {}
    for i, v in ipairs(letters) do
        if v then
            unshapedWord[#unshapedWord + 1] = v
        end
    end

    return unshapedWord
end

local function decomposeWord(word)
    local harakat = {}
    local harakatPositions = {}
    local letters = {}
    local letterPositions = {}

    for i, c in ipairs(word) do
        if Harakat[c] then
            harakatPositions[#harakatPositions+1] = i
            harakat[#harakat+1] = c
        else
            letterPositions[#letterPositions+1] = i
            letters[#letters+1] = c
        end
    end

    return {
        reshaped = word,
        harakat = harakat,
        letters = letters,
        harakatPositions = harakatPositions,
        letterPositions  = letterPositions
    }
end

local function reshapeDecomposed(word)
    if not word[1] then return word end
    if not word[2] then return {getReshapedGlyph(word[1], 2)} end

    local reshaped = {}
    local before, after
    for i, c in ipairs(word) do
        before, after = false, false

        if i ~= #word then
            after = (getGlyphType(word[i]) == 4)
        end

        if i ~= 1 then
            before = (getGlyphType(word[i-1]) == 4)
        end

        if after and before then
            reshaped[#reshaped+1] = getReshapedGlyph(word[i], 4)
        elseif after and not before then
            reshaped[#reshaped+1] = getReshapedGlyph(word[i], 3)
        elseif not after and before then
            reshaped[#reshaped+1] = getReshapedGlyph(word[i], 5)
        else
            reshaped[#reshaped+1] = getReshapedGlyph(word[i], 2)
        end
    end

    return reshaped
end

local function reconstructDecomposed(word, reshaped)
    local withHarakat = {}

    for i, pos in ipairs(word.letterPositions) do
        withHarakat[pos] = reshaped[i]
    end

    for i, pos in ipairs(word.harakatPositions) do
        withHarakat[pos] = word.harakat[i]
    end

    return withHarakat
end

local function getReshapedWord(word)
    word = replaceLamAlef(word)
    word = replaceShadda(word)

    local decomposed, result = decomposeWord(word), {}
    if decomposed.letters[1] then
        result = reshapeDecomposed(decomposed.letters)
    end

    return reconstructDecomposed(decomposed, result)
end

local function getWords(sentence)
    local currWord = {}
    local words = {currWord}

    for i, c in ipairs(sentence) do
        if c == 0x20 or c == 0xA or c == 0x9 or c == 0xD or c == 0x0 then
            currWord = {}
            words[#words+1] = currWord
        else
            currWord[#currWord+1] = c
        end
    end

    return words
end

local function isArabicCharacter(c)
    return Glyphs[c] or Harakat[c]
end

local function hasArabicLetters(word)
    for i, c in ipairs(word) do
        if isArabicCharacter(c) then return true end
    end

    return false
end

local function isArabicWord(word)
    for i, c in ipairs(word) do
        if not isArabicCharacter(c) then return false end
    end

    return true
end

local function getWordsFromMixedWord(word)
    local words = {}
    local tempWord = {}

    for i, c in ipairs(word) do
        if isArabicCharacter(c) then
            if tempWord[1] and not isArabicWord(tempWord) then
                words[#words+1] = tempWord
                tempWord = {c}
            else
                tempWord[#tempWord+1] = c
            end
        else
            if tempWord[1] and isArabicWord(tempWord) then
                words[#words+1] = tempWord
                tempWord = {c}
            else
                tempWord[#tempWord+1] = c
            end
        end
    end

    if tempWord[1] then words[#words+1] = tempWord end

    return words
end

local function reverseWord(word)
    local reversed = {}
    for i = #word, 1, -1 do
        reversed[#reversed+1] = word[i]
    end
    return reversed
end

local function reshapeWords(words, nospaces)
    local nonArabicPositions = {}

    for i, word in ipairs(words) do
        if hasArabicLetters(word) then
            -- Reverse word ordering, mixed words don't have to reverse order
            if #nonArabicPositions > 0 and not nospaces then
                for i = 1, #nonArabicPositions / 2 do
                    local pos = nonArabicPositions[i]
                    local oppositePos = nonArabicPositions[#nonArabicPositions-i+1]
                    local temp = words[pos]
                    words[pos] = words[oppositePos]
                    words[oppositePos] = temp
                end
                nonArabicPositions = {}
            end

            if isArabicWord(word) then
                words[i] = getReshapedWord(word)
            else
                words[i] = reshapeWords(getWordsFromMixedWord(word), true)
            end
        else
            words[i] = reverseWord(word)
            nonArabicPositions[#nonArabicPositions+1] = i
        end
    end

    if #nonArabicPositions > 0 and not nospaces then
        for i = 1, #nonArabicPositions / 2 do
            local pos = nonArabicPositions[i]
            local oppositePos = nonArabicPositions[#nonArabicPositions-i+1]

            local temp = words[pos]
            words[pos] = words[oppositePos]
            words[oppositePos] = temp
        end
    end

    sentence = {}
    for i, word in ipairs(words) do
        for j, c in ipairs(word) do
            sentence[#sentence+1] = c
        end
        if i < #words and words[i+1][1] and not nospaces then
            sentence[#sentence+1] = 0x20
        end
    end

    return sentence
end

local function reshapeSentence(sentence)
    if not sentence[1] then return {} end
    
    return reshapeWords(getWords(sentence))
end

local function reshape(str, utf8)
    if type(str) == 'string' then
        str = require('nx.util.unicode').utf8To32(str)
    end

    if not str[1] then return {} end

    local lines = {{}}

    -- Split into lines
    local currLine = lines[1]
    for i, v in ipairs(str) do
        if v == 0xA then -- '\n'
            currLine = {}
            lines[#lines+1] = currLine
        else
            currLine[#currLine+1] = v
        end
    end

    -- Process and join lines
    str = {}
    for i, v in ipairs(lines) do
        -- Process line
        lines[i] = reshapeSentence(lines[i])

        -- Add it to the string
        for j, w in ipairs(lines[i]) do
            str[#str+1] = w
        end

        -- Add a '\n' if not the last line
        if i ~= #lines then
            str[#str+1] = 0xA
        end
    end

    return utf8 and require('nx.util.unicode').utf32To8(str) or str
end

return reshape
