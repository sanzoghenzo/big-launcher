#pragma once

#include <SDL3/SDL.h>

namespace BL {
    class Texture {
    protected:
        bool update_buffer = true;
        bool update_tx_coords = true;
        SDL_FRect pos{};
        SDL_FRect tex_coords{};

    public:
        Texture() = default;
        Texture(float w, float h): pos({0.f, 0.f, w, h}), tex_coords(0.f, 0.f, w, h) {}
        virtual ~Texture() = default;

        virtual void set_color_mod(const SDL_Color &color) = 0;
        void set_x(float x) { pos.x = x; update_buffer = true; }
        void set_y(float y) { pos.y = y; update_buffer = true; }
        void set_w(float w) { pos.w = w; update_buffer = true; }
        void set_h(float h) { pos.h = h; update_buffer = true; }
        void update_pos(const SDL_FRect &pos) { this->pos = pos; update_buffer = true; }
        float get_x() const { return pos.x; }
        float get_y() const { return pos.y; }
        float get_w() const { return pos.w; }
        float get_h() const { return pos.h; }
        const SDL_FRect& get_pos() const { return pos; }
        const SDL_FRect& get_tex_coords() const { return tex_coords; }

    };

    class Renderer {
    private:
        SDL_Window &window;

    protected:
        Renderer(SDL_Window &window): window(window) {}

    public:
        virtual ~Renderer() = default;

        virtual void set_draw_color(const SDL_Color &color) = 0;
        virtual void clear() = 0;
        virtual void present() = 0;
        virtual void set_clip_rect(SDL_Rect &rect) = 0;
        virtual void disable_clip() = 0;
        virtual void draw(Texture &texture) = 0;

        virtual Texture* create_texture(SDL_Surface &surface) = 0;
        virtual Texture* create_texture(SDL_Surface &surface, int w, int h) = 0;
        virtual Texture* create_texture(int w, int h) = 0;
        virtual void composit_texture(const Texture &src, const Texture &dst, SDL_FRect *coords) = 0;
        virtual void set_render_scale(float scale_w, float scale_h) {}
        virtual void set_logical_representation(int w, int h) {}

    };
}
