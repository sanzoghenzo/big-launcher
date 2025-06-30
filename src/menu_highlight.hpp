#pragma once

#include "drawable.hpp"

// Menu highlight geometry
#define HIGHLIGHT_THICKNESS 0.5f
#define HIGHLIGHT_INNER_SPACING 0.25f
#define MENU_HIGHLIGHT_RX 0.02f

namespace BL {
    class SVGRasterizer;
    class MenuHighlight: public Drawable {
    public:
        void render_surface(SVGRasterizer &rasterizer, int w, int h, int t);
        void render_texture();

        MenuHighlight(): Drawable() {}
        ~MenuHighlight() = default;

    };
}

