#include <unordered_map>
#include <algorithm>

#include "external/ini.h"
#include "logger.hpp"

#include "config.hpp"
#include "sound.hpp"
#include "screensaver.hpp"
#include "util.hpp"

#define MATCH(a,b) !strcmp(a,b)
namespace BL {
    extern "C" int handler(void* user, const char* section, const char* name, const char* value);
    struct GamepadInfo {
        GamepadControl::Type type;
        GamepadControl::Direction direction;
        int index;
    };
}

extern BL::Config config;

int BL::handler(void* user, const char* section, const char* name, const char* value)
{
    if (MATCH(section, "Settings")) {
        if (MATCH(name, "MouseSelect"))
            config.add_bool(value, config.mouse_select);
        else if (MATCH(name, "SidebarHighlightColor"))
            BL::hex_to_color(value, config.sidebar_highlight_color);
        else if (MATCH(name, "SidebarTextColor"))
            BL::hex_to_color(value, config.sidebar_text_color);
        else if (MATCH(name, "SidebarTextSelectedColor"))
            BL::hex_to_color(value, config.sidebar_text_color_highlighted);
        else if (MATCH(name, "MenuHighlightColor"))
            BL::hex_to_color(value, config.menu_highlight_color);
        else if (MATCH(name, "BackgroundImage"))
            config.add_path(value, config.background_image_path);
    }

    else if (MATCH(section, "Sound")) {
        if (MATCH(name, "Enabled"))
            config.add_bool(value, config.sound_enabled);
        else if (MATCH(name, "Volume"))
            config.add_int(value, config.sound_volume);
    }

    else if (MATCH(section, "Screensaver")) {
        if (MATCH(name, "Enabled"))
            config.add_bool(value, config.screensaver_enabled);
        else if (MATCH(name, "IdleTime"))
            config.add_time(value, config.screensaver_idle_time, MIN_SCREENSAVER_IDLE_TIME, MAX_SCREENSAVER_IDLE_TIME);
        else if (MATCH(name, "Intensity"))
            config.add_percent<Uint8>(value, config.screensaver_intensity, 255, 0.1f, 1.f);
    }

    else if (MATCH(section, "Hotkeys")) {
        config.hotkey_list.add(value);
    }

    else if (MATCH(section, "Gamepad")) {
        if (MATCH(name, "Enabled"))
            config.add_bool(value, config.gamepad_enabled);
        else if (MATCH(name, "DeviceIndex"))
            config.add_int(value, config.gamepad_index);
        else if (MATCH(name, "MappingsFile"))
            config.add_path(value, config.gamepad_mappings_file);
        else
            config.add_gamepad_control(name, value);
    }
    return 0;
}

BL::Config::Config(): sound_volume(BL::MAX_VOLUME) {}

void BL::Config::parse(const std::string &file)
{
    BL::logger::debug("Parsing config file '{}'", file);
    if (ini_parse(file.c_str(), BL::handler, nullptr) < 0)
        BL::logger::critical("Failed to parse config file");
    BL::logger::debug("Sucessfully parsed config file");
}

void BL::Config::add_bool(const char *value, bool &out)
{
    if (MATCH(value, "true") || MATCH(value, "True"))
        out = true;
    else if (MATCH(value, "false") || MATCH(value, "False"))
        out = false;
}

void BL::Config::add_int(const char *value, int &out)
{
    int x = std::stoi(value);
    if (x || *value == '0')
        out = x;
}

void BL::Config::add_time(const char *value, Uint32 &out, Uint32 min, Uint32 max)
{
    int x = std::stoi(value) * 1000;
    if ((x || *value == '0') && x >= min && x <= max)
        out = x;
}

void BL::Config::add_path(const char *value, std::string &out)
{
    out = value;

    // Remove double quotes
    if (*out.begin() == '"' && *(out.end() - 1) == '"')
        out = out.substr(1, out.size() - 2);
}

template <typename T>
void BL::Config::add_percent(const char *value, T &out, T ref, float min, float max)
{
    std::string_view string = value;
    if (string.back() != '%')
        return;
    float percent = atof(std::string(string, 0, string.size() - 1).c_str()) / 100.0f;
    if (percent == 0.0f && strcmp(value, "0%"))
        return;
    if (percent < min)
        percent = min;
    else if (percent > max)
        percent = max;
    
    out = static_cast<T>(percent * static_cast<float>(ref));
}

void BL::Config::add_gamepad_control(const char *key, const char *value)
{
      static const std::unordered_map<std::string, BL::GamepadInfo> infos = {
        {"LStickX-",         {GamepadControl::Type::LSTICK,  GamepadControl::Direction::XM,   SDL_GAMEPAD_AXIS_LEFTX}},
        {"LStickX+",         {GamepadControl::Type::LSTICK,  GamepadControl::Direction::XP,   SDL_GAMEPAD_AXIS_LEFTX}},
        {"LStickY-",         {GamepadControl::Type::LSTICK,  GamepadControl::Direction::YM,   SDL_GAMEPAD_AXIS_LEFTY}},
        {"LStickY+",         {GamepadControl::Type::LSTICK,  GamepadControl::Direction::YP,   SDL_GAMEPAD_AXIS_LEFTY}},
        {"RStickX-",         {GamepadControl::Type::RSTICK,  GamepadControl::Direction::XM,   SDL_GAMEPAD_AXIS_RIGHTX}},
        {"RStickX+",         {GamepadControl::Type::RSTICK,  GamepadControl::Direction::XP,   SDL_GAMEPAD_AXIS_RIGHTX}},
        {"RStickY-",         {GamepadControl::Type::RSTICK,  GamepadControl::Direction::YM,   SDL_GAMEPAD_AXIS_RIGHTY}},
        {"RStickY+",         {GamepadControl::Type::RSTICK,  GamepadControl::Direction::YP,   SDL_GAMEPAD_AXIS_RIGHTY}},
        {"LTrigger",         {GamepadControl::Type::TRIGGER, GamepadControl::Direction::NONE, SDL_GAMEPAD_AXIS_LEFT_TRIGGER}},
        {"RTrigger",         {GamepadControl::Type::TRIGGER, GamepadControl::Direction::NONE, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER}},
        {"ButtonA",          {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_SOUTH}},
        {"ButtonB",          {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_EAST}},
        {"ButtonX",          {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_WEST}},
        {"ButtonY",          {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_NORTH}},
        {"ButtonBack",       {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_BACK}},
        {"ButtonGuide",      {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_GUIDE}},
        {"ButtonStart",      {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_START}},
        {"ButtonLStick",     {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_LEFT_STICK}},
        {"ButtonRStick",     {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_RIGHT_STICK}},
        {"ButtonLShoulder",  {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER}},
        {"ButtonRShoulder",  {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER}},
        {"ButtonDPadUp",     {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_DPAD_UP}},
        {"ButtonDPadDown",   {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_DPAD_DOWN}},
        {"ButtonDPadLeft",   {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_DPAD_LEFT}},
        {"ButtonDPadRight",  {GamepadControl::Type::BUTTON,  GamepadControl::Direction::NONE, SDL_GAMEPAD_BUTTON_DPAD_RIGHT}}
    };

    static const SDL_GamepadAxis opposing_axes[] = {
        SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_LEFTX,
        SDL_GAMEPAD_AXIS_RIGHTY,
        SDL_GAMEPAD_AXIS_RIGHTX,
    };

    auto it = infos.find(key);
    if (it != infos.end()) {
        const GamepadInfo &info = it->second;
        config.gamepad_controls.emplace_back(info.type, info.index, info.direction, it->first, value);

        // Add control stick for axis if it doesn't already exist
        if (info.type == GamepadControl::Type::LSTICK || info.type == GamepadControl::Type::RSTICK) {
            SDL_GamepadAxis opposing_axis = opposing_axes[info.index];
            auto a = std::find_if(config.gamepad_sticks.begin(),
                        config.gamepad_sticks.end(),
                        [&](auto &stick) { return stick.axes[0] == info.index || stick.axes[1] == info.index; }
                     );
            if (a == config.gamepad_sticks.end())
                config.gamepad_sticks.push_back(BL::GamepadStick(info.type, {(SDL_GamepadAxis) info.index, opposing_axis}));
        }
    }
}
