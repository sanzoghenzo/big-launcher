#include <SDL3/SDL.h>
#include "screensaver.hpp"
#include "image.hpp"
#include "config.hpp"
#include "main.hpp"

extern BL::Config config;

void BL::Screensaver::render_surface()
{
    surface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    Uint32 color = SDL_MapSurfaceRGBA(surface, 0, 0, 0, 0xFF);
    SDL_FillSurfaceRect(surface, nullptr, color);
    opacity_change_rate = static_cast<float>(config.screensaver_intensity) / SCREENSAVER_TRANSITION_TIME;
    pos = {0, 0, static_cast<float>(w), static_cast<float>(h)};
}

void BL::Screensaver::render_texture()
{
    texture = renderer->create_texture(*surface);
    BL::free_surface(surface);
    surface = nullptr;
}

void BL::Screensaver::update()
{
    if (!active) {
        if (launcher.time_since_last_input() > config.screensaver_idle_time) {
            active = true;
            transitioning = true;
            current_ticks = launcher.current_time();
            texture->set_color_mod({0xFF, 0xFF, 0xFF, 0});
        }
    }
    else {
        if (transitioning) {
            float delta = (static_cast<float>((launcher.current_time() - current_ticks))) * opacity_change_rate;
            opacity += delta;
            Uint8 op = static_cast<Uint8>(std::round(opacity));
            if (op >= config.screensaver_intensity) {
                op = config.screensaver_intensity;
                transitioning = false;
                opacity = 0.f;
            }
            texture->set_color_mod({0xFF, 0xFF, 0xFF, op});
            current_ticks = launcher.current_time();
        }
        if ((launcher.time_since_last_input()) < config.screensaver_idle_time) {
            active = false;
            transitioning = false;
            opacity = 0.f;
        }
    }
}
