/*
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
*/

#include "../config.hpp"

#include <SDL2/SDL.h>
#include <map>
#include <cstring>
#include <string>
#include <sstream>

using NxGamepad = SDL_GameController;

static std::map<std::string, bool> recentGUIDs;

NX_EXPORT NxGamepad* nxGamepadOpen(int id)
{
    NxGamepad* gamepad = SDL_GameControllerOpen(id - 1);
    if (gamepad) {
        // TODO add GUID for recentGUIDs
        auto guid = SDL_JoystickGetDeviceGUID(id);

        std::string guidStr(64, '\0');
        SDL_JoystickGetGUIDString(guid, &guidStr[0], guidStr.size());
        recentGUIDs[guidStr.data()] = true;
    }

    return gamepad;
}

NX_EXPORT void nxGamepadClose(NxGamepad* gamepad)
{
    SDL_GameControllerClose(gamepad);
}

NX_EXPORT bool nxGamepadButtonDown(NxGamepad* gamepad, int btn)
{
    return SDL_GameControllerGetButton(gamepad, static_cast<SDL_GameControllerButton>(btn - 1));
}

NX_EXPORT double nxGamepadGetAxis(NxGamepad* gamepad, int axis)
{
    return SDL_GameControllerGetAxis(gamepad, static_cast<SDL_GameControllerAxis>(axis - 1));
}

NX_EXPORT bool nxGamepadAddMapping(const char* mappingStr)
{
    return SDL_GameControllerAddMapping(mappingStr) != -1;
}

NX_EXPORT const char* nxGamepadGetMapping(const char* guidStr)
{
    auto guid = SDL_JoystickGetGUIDFromString(guidStr);
    return SDL_GameControllerMappingForGUID(guid);
}

NX_EXPORT bool nxGamepadAddMappings(const char* data)
{
    // Add to recent GUIDs
    std::string platformStr = std::string("platform:") + SDL_GetPlatform();
    std::stringstream ss(data);
    std::string mapping;
    while (std::getline(ss, mapping)) {
        // Empty or comment line
        if (mapping.empty() || mapping[0] == '#') continue;

        // Check if right platform
        if (mapping.find(platformStr) == std::string::npos) continue;

        // Get the GUID
        std::string guid = mapping.substr(0, mapping.find_first_of(','));

        // Check if the mapping is even valid
        if (guid.size() == mapping.size()) continue;

        recentGUIDs[guid] = true;
    }

    SDL_RWops* rw = SDL_RWFromConstMem(data, strlen(data));
    return SDL_GameControllerAddMappingsFromRW(rw, 1) != -1;
}

NX_EXPORT const char* nxGamepadGetMappings()
{
    static std::string str;
    std::stringstream ss;

    for (auto& it : recentGUIDs) {
        const char* mapping = nxGamepadGetMapping(it.first.data());
        if (mapping) ss << mapping << '\n';
    }

    str = ss.str();
    return str.data();
}
