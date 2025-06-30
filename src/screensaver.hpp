#pragma once

#include <SDL3/SDL.h>

#include "drawable.hpp"

#define SCREENSAVER_TRANSITION_TIME 2000.f
#define MIN_SCREENSAVER_IDLE_TIME 5
#define MAX_SCREENSAVER_IDLE_TIME 60000

namespace BL {
    class Renderer;
    class Launcher;
    class Screensaver : public Drawable {
        private:
            float opacity = 0.f;
            float opacity_change_rate;
            SDL_Surface *surface = nullptr;
            Uint32 current_ticks;
            int w;
            int h;
            bool active = false;
            bool transitioning = false;
            Launcher &launcher;
        
        public:
            Screensaver(int w, int h, Launcher &launcher): Drawable(), w(w), h(h), launcher(launcher) {}
            ~Screensaver() = default;
            void render_surface();
            void render_texture();
            void update();
            bool is_active() const { return active; }
            bool is_transitioning() const { return transitioning; }

    };
}
