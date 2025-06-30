#include <algorithm>
#include <cmath>
#include "logger.hpp"
#include "gamepad.hpp"
#include "config.hpp"
#include "main.hpp"

extern BL::Config config;

namespace BL {
    constexpr int GAMEPAD_DEADZONE = 15000;
    constexpr int GAMEPAD_REPEAT_DELAY = 500;
    constexpr int GAMEPAD_REPEAT_INTERVAL = 25;
}

BL::Gamepad::Gamepad(int refresh_period, const std::string &gamepad_mappings_file, BL::Launcher &launcher):
    launcher(launcher)
{
    BL::logger::debug("Initializing game controller subsystem...");
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        BL::logger::error("Could not initialize game controller subsystem (SDL Error: {})", SDL_GetError());
        throw std::runtime_error("");
    }
    BL::logger::debug("Successfully initialized game controller subsystem");

    delay_period = BL::GAMEPAD_REPEAT_DELAY / refresh_period;
    repeat_period = BL::GAMEPAD_REPEAT_INTERVAL / refresh_period;

    if (!gamepad_mappings_file.empty()) {
        if (SDL_AddGamepadMappingsFromFile(gamepad_mappings_file.c_str()) < 0) {
            BL::logger::error("Could not load gamepad mappings from file '{}'", 
                gamepad_mappings_file.c_str()
            );
        }
    }
}

BL::Gamepad::~Gamepad()
{
    for (auto& [id, gamepad] : controllers)
        SDL_CloseGamepad(gamepad);
}

void BL::Gamepad::add(int id)
{
    SDL_Gamepad *gamepad = SDL_OpenGamepad(id);
    if (gamepad)
        controllers[id] = gamepad;
}

void BL::Gamepad::remove(int id)
{
    auto it = controllers.find(id);
    if (it != controllers.end()) {
        SDL_CloseGamepad(it->second);
        controllers.erase(it);
    }
}

void BL::Gamepad::connect()
{
    for (auto& [id, gamepad] : controllers)
        gamepad = SDL_OpenGamepad(id);
}

void BL::Gamepad::disconnect()
{
    for (auto& [id, gamepad] : controllers) {
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
    }
}

bool BL::Gamepad::poll()
{
    bool ret = false;
    // Poll sticks
    if (!config.gamepad_sticks.empty()) {
        AxisType axis_type;
        for (auto stick = config.gamepad_sticks.begin(); stick != config.gamepad_sticks.end() && !selected_axis; ++stick) {
            for (auto controller = controllers.begin(); controller != controllers.end() && !selected_axis; ++controller) {
                // Poll individual axes in stick
                for (auto axis : stick->axes) {
                    axis_type = (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX) ? AxisType::X : AxisType::Y;
                    axis_values[stick->type][axis_type] = SDL_GetGamepadAxis(controller->second, axis);
                }
                AxisType max_axis = (abs(axis_values[stick->type][static_cast<int>(AxisType::X)]) >= abs(axis_values[stick->type][static_cast<int>(AxisType::Y)])) ? AxisType::X : AxisType::Y;
                if (abs(axis_values[stick->type][max_axis]) < BL::GAMEPAD_DEADZONE)
                    continue;
                
                // Determine direction of stick movement
                GamepadControl::Direction direction;
                if (max_axis == AxisType::X) {
                    if (axis_values[stick->type][static_cast<int>(AxisType::X)] < 0)
                        direction = GamepadControl::Direction::XM;
                    else
                        direction = GamepadControl::Direction::XP;
                }
                else if (max_axis == AxisType::Y) {
                    if (axis_values[stick->type][static_cast<int>(AxisType::Y)] < 0)
                        direction = GamepadControl::Direction::YM;
                    else
                        direction = GamepadControl::Direction::YP;
                }

                // Save set selected axis if stick press is within range of a control
                auto it = std::find_if(config.gamepad_controls.begin(),
                            config.gamepad_controls.end(),
                            [&](auto control){return stick->type == control.type && direction == control.direction;}
                        );
                if (it != config.gamepad_controls.end()) {
                    AxisType min_axis = (max_axis == AxisType::X) ? AxisType::Y : AxisType::X;
                    if (abs(axis_values[stick->type][min_axis]) < abs((int) std::round((float) axis_values[stick->type][max_axis] * MAX_OPPOSING)))
                        selected_axis = &*it;
                }
            }
        }
    }


    // Check each control
    for (GamepadControl &control : config.gamepad_controls) {
        if (control.type == GamepadControl::Type::LSTICK || control.type == GamepadControl::Type::RSTICK) {
            if (&control == selected_axis) { // We already polled for the stick
                control.repeat++;
                selected_axis = nullptr;
            }
            else
                control.repeat = 0;
        }

        // Poll buttons
        else {
            bool pressed = false;
            for (auto [id, gamepad] : controllers) {
                if ((control.type == GamepadControl::Type::BUTTON && SDL_GetGamepadButton(gamepad, (SDL_GamepadButton) control.index)) ||
                (control.type == GamepadControl::Type::TRIGGER && SDL_GetGamepadAxis(gamepad, (SDL_GamepadAxis) control.index) > BL::GAMEPAD_DEADZONE)) {
                    pressed = true;
                    break;
                }
            }
            control.repeat = pressed ? control.repeat + 1 : 0;
        }

        if (control.repeat == 1) {
            BL::logger::debug("Gamepad {} detected", control.label);
            ret = true;
            launcher.execute_command(control.command);

        }
        else if (control.repeat == delay_period) {
            launcher.execute_command(control.command);
            control.repeat -= repeat_period;
        }
    }
    return ret;
}
