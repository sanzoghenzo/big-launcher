#pragma once

#include <cassert>

#include <SDL3/SDL.h>
#include "object.hpp"
#include "renderer.hpp"
#include "image.hpp"

namespace BL {
    class Texture;
    class Renderer;
    class Drawable: public Object {
    protected:
        Renderer *renderer = nullptr;
        SDL_Surface *surface = nullptr;
        Texture *texture = nullptr;
        SDL_FRect pos{};
        float shadow_offset = 0.f;
        bool updated_pos = true;
        Drawable(): Object() {}

    public:
        ~Drawable()
        {
            BL::free_surface(surface);
            delete texture;
        }
        void draw()
        { 
            assert(renderer);
            assert(texture);
            if (updated_pos)
                texture->update_pos(pos);
            updated_pos = false;
            renderer->draw(*texture);

        }
        const Texture* get_texture() const { return texture; }
        void set_renderer(Renderer &renderer) { this->renderer = &renderer; }
        const SDL_FRect& get_pos() const { return pos; }
        float get_x() const override final { return pos.x + shadow_offset; }
        void set_x(float x) override final { pos.x = x - shadow_offset; updated_pos = true; }
        float get_y() const override final { return pos.y + shadow_offset; }
        void set_y(float y) override final { pos.y = y - shadow_offset; updated_pos = true; }
        float get_w() const override final { return pos.w - 2*shadow_offset; }
        void set_w(float w) { pos.w = w + 2*shadow_offset; updated_pos = true; }
        float get_h() const override final { return pos.h - 2*shadow_offset; }
        void set_h(float h) { pos.h = h + 2*shadow_offset; updated_pos = true; }
        void inc_x(float x) override final { pos.x += x; updated_pos = true; }
        void dec_x(float x) override final { pos.x -= x; updated_pos = true; }
        void inc_y(float y) override final { pos.y += y; updated_pos = true; }
        void dec_y(float y) override final { pos.y -= y; updated_pos = true; }
        float get_shadow_offset() const { return shadow_offset; }
        void set_shadow_offset(float shadow_offset) { this->shadow_offset = shadow_offset; }
    };
}
