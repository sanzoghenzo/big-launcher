#pragma once

#include <vector>
#include <string>

#include <SDL3/SDL.h>
#include <libxml/parser.h>

#include "drawable.hpp"
#include "util.hpp"

namespace BL {
    constexpr float CARD_ASPECT_RATIO = 4.f / 3.f;
    class Texture;
    class SVGRasterizer;
    class Renderer;
    class MenuEntry: public Drawable {
    public:
        enum CardType {
            CUSTOM,
            GENERATED
        };

    private:
        CardType card_type;
        std::string title;
        std::string command;
        SDL_Color background_color { 0xFF, 0xFF, 0xFF, 0xFF };
        std::string path; // doubles for both card path and background in generated mode
        std::string icon_path;
        SDL_Surface *icon_surface = nullptr;
        SDL_FRect icon_rect;
        float icon_margin;
        bool card_error = false;
    
    public:
        MenuEntry(const std::string &title, const std::string &command);
        ~MenuEntry();

        void set_card(const std::string &path);
        void set_card(SDL_Color &background_color, const std::string &path);
        void set_card(const std::string &background_path, const std::string &icon_path);
        void set_card_error(bool card_error) { this->card_error = card_error; }
        bool get_card_error() const { return card_error; }
        void set_margin(const char *value);
        bool render_surface(SVGRasterizer &rasterizer, float w, float h, float shadow_offset);
        void render_texture(Texture &shadow_texture, float card_w, float card_h, float shadow_offset);
        const std::string& get_command() const { return command; }
        const std::string& get_title() const { return title; }
    };

    class Menu: public Object {
    private:
        std::string title;
        void add_entry(xmlNodePtr node);
        std::vector<MenuEntry> entry_list;
        int row = 0;
        int column = 0;
        int total_rows = 0;
        int max_columns = 0;
        int height;
        int nb_columns;
        float x_advance = 0.f;
        float y_advance = 0.f;
        int shift_count = 0;
        Texture *error_texture = nullptr; // owned by Layout
        std::vector<MenuEntry>::iterator current_entry;
        Renderer *renderer = nullptr;

    public:
        Menu(const std::string &title, int nb_columns);
        ~Menu();
        bool parse(xmlNodePtr node);
        const std::string& get_title() const { return title; }
        size_t num_entries() { return entry_list.size(); }
        void set_renderer(Renderer &renderer) { this->renderer = &renderer; }
        bool render_surfaces(BL::SVGRasterizer &rasterizer, float shadow_offset, float w, float h);
        void render_card_textures(Texture &card_shadow_texture, float shadow_offset, float card_w, float card_h);
        void draw();
        void print_entries();
        void set_error_texture(Texture &error_texture) { this-> error_texture = &error_texture; }
        MenuEntry& get_current_entry() { return *current_entry; }
        int get_row() const { return row; }
        void inc_row() { row++; current_entry += max_columns; }
        void dec_row() { row--; current_entry -= max_columns; }
        void reset_row() { row = 0; current_entry = entry_list.begin(); }
        int get_column() const { return column; }
        void inc_column() { column++; current_entry++; }
        void dec_column() { column--; current_entry--; }
        int get_total_rows() const { return total_rows; }
        int get_max_columns() const { return max_columns; }
        int get_shift_count() const { return shift_count; }
        void inc_shift_count() { shift_count++; }
        void dec_shift_count() { shift_count--; }
        void reset_shift_count() { shift_count = 0; }
        float get_x() const override final { return entry_list[0].get_x(); }
        void set_x(float x) override final;
        float get_y() const override final { return entry_list[0].get_y(); }
        void set_y(float y) override final;
        float get_w() const override final { return entry_list[max_columns - 1].get_x() + entry_list[max_columns - 1].get_w() - entry_list[0].get_x(); }
        float get_h() const override final { return entry_list.back().get_y() + entry_list.back().get_h() - entry_list[0].get_y(); }
        void inc_x(float x) override final;
        void dec_x(float x) override final;
        void inc_y(float y) override final;
        void dec_y(float y) override final;
        void set_x_advance(float x_advance) { this->x_advance = x_advance; }
        void set_y_advance(float y_advance) { this->y_advance = y_advance; }
        bool is_visible(float y1, float y2)
        {
            float menu_y1 = get_y();
            float menu_y2 = get_y() + get_h();
            return (menu_y2 >= y1 && menu_y2 <= y2) || (menu_y1 <= y2 && menu_y1 >= y1) || (menu_y1 < y1 && menu_y2 > y2);
        }
    };
}
