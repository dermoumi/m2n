#include "../config.hpp"
#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <sstream>
#include "../log.hpp"

using NxGamepad = SDL_GameController;

namespace
{
    std::map<std::string, bool> recentGUIDs;
}

extern "C"
{
    NX_EXPORT NxGamepad* nxGamepadOpen(int id)
    {
        NxGamepad* gamepad = SDL_GameControllerOpen(id - 1);
        if (gamepad) {
            // TODO add GUID for recentGUIDs
            auto guid = SDL_JoystickGetDeviceGUID(id);

            std::string guidStr(64, '\0');
            SDL_JoystickGetGUIDString(guid, &guidStr[0], guidStr.size());
            recentGUIDs[guidStr.data()] = true;
            Log::info("Registered: '%s'", guidStr.data());
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
            auto guid = SDL_JoystickGetGUIDFromString(it.first.data());
            ss << SDL_GameControllerMappingForGUID(guid) << '\n';
        }

        str = ss.str();
        return str.data();
    }
}