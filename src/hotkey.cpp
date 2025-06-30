#include <string_view>
#include <SDL3/SDL.h>
#include "hotkey.hpp"
#include "platform/platform.hpp"

void BL::HotkeyList::add(const char *value)
{
    std::string_view string = value;
    if (string.front() != '#')
        return;
    size_t pos = string.find_first_of(";");
    if (pos == std::string::npos || pos == (string.size() - 1))
        return;

    std::string keycode_s(string, 1, pos);
    SDL_Keycode keycode = (SDL_Keycode) strtol(keycode_s.c_str(), nullptr, 16);
    if (!keycode)
        return;
    std::string_view command((char*) value + pos + 1);
    
#ifdef _WIN32
    if (command == ":exit") {
        set_exit_hotkey(keycode);
        return;
    }
#endif

    list.emplace_back(keycode, (char*) value + pos + 1);
}
