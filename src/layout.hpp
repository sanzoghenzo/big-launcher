#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include "object.hpp"

namespace BL {
    class SVGRasterizer;
    class Menu;
    class MenuEntry;
    class MenuHighlight;
    class SidebarHighlight;
    class SidebarEntry;
    class Font;
    class Texture;
    class Screensaver;
    class Launcher;
    class Renderer;
    class Layout {
        private:
            enum SelectionMode {
                SIDEBAR,
                MENU
            };
            enum Direction {
                UP,
                DOWN,
                LEFT,
                RIGHT
            };

            struct Press {
                MenuEntry *entry;
                SDL_FRect original_rect;
                float total;
                float current;
                float velocity;
                Direction direction;
                float aspect_ratio;
                Uint64 ticks;

                Press(MenuEntry &entry);
                ~Press() = default;
            };

            struct Shift {
                enum Type {
                    SIDEBAR,
                    MENU,
                    HIGHLIGHT
                };
                enum Method {
                    ABS,
                    REL
                };
                Type type;
                Method method;
                Direction direction;
                Uint32 ticks;
                float velocity;
                float total;
                float target;
                std::vector<Object*> objects;
                Shift(Type type, Method method, Direction direction, float velocity, float target, const std::vector<Object*> &objects):
                    type(type), method(method), direction(direction), ticks(SDL_GetTicks()), velocity(velocity), total(0.f), target(target), objects(objects) {}
                ~Shift() = default;
            };

            // Layout class members
            SVGRasterizer *rasterizer;
            Renderer *renderer = nullptr;
            int screen_width;
            int screen_height;
            float f_screen_width;
            float f_screen_height;

            SDL_Surface *background_surface = nullptr;
            Texture *background_texture = nullptr;

            bool card_error = false;
            SDL_Surface *error_bg = nullptr;
            SDL_Surface *error_icon = nullptr;
            SDL_FRect error_icon_rect;
            Texture *error_texture = nullptr;

            // States
            std::vector<Shift> shift_queue;
            std::vector<Press> press_queue;
            SelectionMode selection_mode = SelectionMode::SIDEBAR;
            Menu *current_menu = nullptr;
            std::vector<Menu> menus;
            std::vector<Object*> menu_objects;

            // Sidebar
            std::vector<SidebarEntry> sidebar_entries;
            std::vector<SidebarEntry>::iterator current_entry;
            SidebarHighlight *sidebar_highlight = nullptr;
            int sidebar_pos = 0;
            float sidebar_y_advance;
            float y_min;
            float y_max;
            int max_sidebar_entries = -1;
            int sidebar_shift_count;
            int num_sidebar_entries = 0;
            SDL_Rect sidebar_highlight_clip{};
            SDL_Rect sidebar_text_clip{};
            SDL_Rect menu_clip{};

            // Menu entry cards
            float card_w;
            float card_h;
            float card_x0;
            float card_y0;
            float card_y_advance;
            float card_spacing;
            int max_rows;
            SDL_Surface *card_shadow = nullptr;
            Texture *card_shadow_texture = nullptr;
            float card_shadow_offset;
            float y_leftover; // vertical spacing between the last row of cards and bottom of screen, when menu is fully extended

            // Highlight
            MenuHighlight *menu_highlight = nullptr;
            float highlight_x0;
            float highlight_y0;
            float highlight_x_advance;
            float highlight_y_advance;

            Screensaver *screensaver = nullptr;
            Launcher &launcher;

            void parse(const std::string &file);
            void load_background();
            void load_menus();
            void load_sidebar();
            void load_menu_entires();
            void load_menu_highlight();
            void render_error_surface();
            void render_error_texture();
            void add_shift(Shift::Type type, Direction direction, float target, float time, const std::vector<Object*> &objects, Shift::Method method = Shift::Method::REL);
            void add_press(MenuEntry &entry) { press_queue.emplace_back(entry); }
            void update_shift();
            void update_press();

        public:
            Layout(const std::string &file, int w, int h, Launcher &launcher);
            ~Layout();

            void load_surfaces();
            void load_textures(Renderer &renderer);
            void update();
            void draw();
            void move_down();
            void move_up();
            void move_left();
            void move_right();
            void select();
    };
}

