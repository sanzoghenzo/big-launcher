#pragma once

#include <vector>
#include <array>
#include <map>

#include <SDL3/SDL.h>

#include "config.hpp"

namespace BL {
    constexpr float PI = 3.14159f;
    constexpr float GAMEPAD_AXIS_RANGE = 60.f; // degrees
    class Launcher;
    class Gamepad {
        private:
            struct Controller {
                SDL_Gamepad *gc = nullptr;
                int device_index;
                bool connected = false;

                Controller(int device_index) : device_index(device_index) { connect(true); }
                void connect(bool raise_error);
                void disconnect();
            };

            enum AxisType{
                X,
                Y,
            };

            Launcher &launcher;
            std::map<int, SDL_Gamepad*> controllers;
            std::array<std::array<int, 2>, 2> axis_values;
            GamepadControl *selected_axis = nullptr;
#ifndef _WIN32 
            constexpr static
#endif
            float MAX_OPPOSING = std::sin((GAMEPAD_AXIS_RANGE / 2.f) * PI / 180.f);

            int delay_period;
            int repeat_period;

        public:
            bool connected;

            Gamepad(int refresh_period, const std::string &gamepad_mappings_file, Launcher &launcher);
            ~Gamepad();
            void add(int id);
            void remove(int id);
            void connect();
            void disconnect();
            bool poll();
    };
}
