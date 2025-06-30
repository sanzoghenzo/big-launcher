#pragma once

#include <string>
#include <variant>

#include <SDL3/SDL.h>

#include "drawable.hpp"

namespace BL {
    class Font;
    class Menu;
    class SidebarEntry: public Drawable {
    private:
        std::string title;
        std::variant<Menu*, std::string> value;
    public:
        SidebarEntry(std::string &&title, std::variant<Menu*, std::string> &&value);
        ~SidebarEntry();
        void render_surface(Font &font, int max_width);
        void render_texture();
        void set_text_color(const SDL_Color &color);
        const std::string& get_title() const { return title; }
        Menu* get_menu() const { auto menu = std::get_if<Menu*>(&value); return menu ? *menu : nullptr; }
        void set_menu(Menu *menu) { value = menu; }
        const std::string* get_command() const { return std::get_if<std::string>(&value); };
    };
}
