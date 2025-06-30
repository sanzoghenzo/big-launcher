#pragma once

#include <SDL3/SDL.h>
#include "drawable.hpp"

namespace BL {
    class SVGRasterizer;
    class SidebarHighlight: public Drawable {
    public:

        void render_surface(SVGRasterizer &rasterizer, float w, float h, int cx);
        void render_texture();
        SidebarHighlight(): Drawable() {}
        ~SidebarHighlight() = default;
            
    };
}

