#pragma once

#include <vector>
#include <string>

#include <SDL3/SDL.h>

namespace BL {
    class Hotkey {
    public:
        SDL_Keycode keycode;
        std::string command;
        Hotkey(SDL_Keycode keycode, std::string_view command) : keycode(keycode), command(command) {}
        ~Hotkey() = default;
    };

    class HotkeyList {
        private:
            std::vector<Hotkey> list;

        public:
            void add(const char *value);
            std::vector<Hotkey>::iterator begin(void) { return list.begin(); }
            std::vector<Hotkey>::iterator end(void) { return list.end(); }
    };
}