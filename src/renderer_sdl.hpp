#pragma once

#include <SDL3/SDL.h>

#include "renderer.hpp"

namespace BL {
    class TextureSDL: public Texture {
    private:
        SDL_Texture *texture;

    public:
        TextureSDL(SDL_Texture *texture): Texture(texture->w, texture->h), texture(texture) {}
        ~TextureSDL() override = default;
        const SDL_Texture* get_texture() const { return texture; }
        void set_color_mod(const SDL_Color &color) override;
    };

    class RendererSDL: public Renderer {
    public:
        RendererSDL(SDL_Window &window);
        ~RendererSDL() override = default;

        //void render(const Texture &texture) override;
        void set_draw_color(const SDL_Color &color) override;
        void clear() override;
        void present() override;
        void draw(Texture &texture) override;

        Texture* create_texture(SDL_Surface &surface) override;
        Texture* create_texture(SDL_Surface &surface, int w, int h) override;
        virtual Texture* create_texture(int w, int h) override;
        void composit_texture(const Texture &src, const Texture &dst, SDL_FRect *coords) override;
        void set_clip_rect(SDL_Rect &rect) override;
        void disable_clip() override;
        void set_render_scale(float scale_w, float scale_h) override;
        void set_logical_representation(int w, int h) override;

    private:
        SDL_Renderer *renderer = nullptr;

    };
}
