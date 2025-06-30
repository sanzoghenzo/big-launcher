#pragma once

#include <string>
#include <array>

#include <SDL3/SDL.h>

#include "hotkey.hpp"

namespace BL {
    constexpr int MAX_VOLUME = 10;
    struct GamepadControl {
        enum Type {
            LSTICK,
            RSTICK,
            BUTTON,
            TRIGGER
        };
        enum Direction {
            NONE = -1,
            XM,
            XP,
            YM,
            YP
        };
        
        Type                       type;
        int                        index;
        GamepadControl::Direction  direction;
        int                        repeat = 0;
        std::string                label;
        std::string                command;
        GamepadControl(Type type, int index, GamepadControl::Direction direction, const std::string &label, const char *cmd) 
        : type(type), index(index), direction(direction), label(label), command(cmd) {}
    };
    struct GamepadStick {
        GamepadControl::Type type;
        std::array<SDL_GamepadAxis, 2> axes;
        GamepadStick(GamepadControl::Type type, std::array<SDL_GamepadAxis, 2> axes): type(type), axes(axes) {}
    };
    class Config {
    public:
        SDL_Color sidebar_highlight_color = {0x00, 0x00, 0xFF, 0xFF};
        SDL_Color sidebar_text_color = {0xFF, 0xFF, 0xFF, 0xFF};
        SDL_Color sidebar_text_color_highlighted = {0xFF, 0xFF, 0xFF, 0xFF};
        SDL_Color menu_highlight_color = {0xFF, 0xFF, 0xFF, 0xFF};
        std::string background_image_path;
        bool mouse_select = false;
        bool debug = false;
        bool sound_enabled = false;
        int sound_volume;
        bool screensaver_enabled = false;
        Uint32 screensaver_idle_time = 900000;
        Uint8 screensaver_intensity = 170;
        bool gamepad_enabled = false;
        int gamepad_index;
        std::string gamepad_mappings_file;
        HotkeyList hotkey_list;
        std::vector<GamepadControl> gamepad_controls;
        std::vector<GamepadStick> gamepad_sticks;
#ifdef DEBUG
        int render_w = 0;
        int render_h = 0;
#endif

        Config();
        ~Config() = default;
        void parse(const std::string &file);
        void add_int(const char *value, int &out);
        void add_string(const char *value, std::string &out);
        void add_bool(const char *value, bool &out);
        void add_path(const char *value, std::string &out);
        void add_time(const char *value, Uint32 &out, Uint32 min, Uint32 max);
        template <typename T>
        void add_percent(const char *value, T &out, T ref, float min = 0.0f, float max = 1.0f);
        void add_gamepad_control(const char *key, const char *value);

    };
}
