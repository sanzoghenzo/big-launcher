#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL.h>

struct BoxShadow{
    float x_offset;
    float y_offset;
    float radius;
    Uint8 alpha;
};

extern "C" {
    struct NSVGimage;
    struct NSVGrasterizer;
}

namespace BL {
    // Shadows
    constexpr float SHADOW_ALPHA = 0.45f;
    constexpr float SHADOW_BLUR_SLOPE = 0.0101212f;
    constexpr float SHADOW_BLUR_INTERCEPT = 8.93f;
    constexpr float SHADOW_OFFSET_SLOPE = 0.008f;
    constexpr float SHADOW_OFFSET_INTERCEPT = 3.15f;
    inline void free_surface(SDL_Surface *s)
    {
        if (s) {
            if(s->flags & SDL_SURFACE_PREALLOCATED) {
                free(s->pixels);
                s->pixels = nullptr;
            }
            SDL_DestroySurface(s);
        }
    }

    class SVGRasterizer {
    private:
        NSVGrasterizer *rasterizer = nullptr;

    public:
        SVGRasterizer();
        ~SVGRasterizer();
        SDL_Surface *rasterize_svg_from_file(const std::string &file, int w, int h);
        SDL_Surface *rasterize_svg(const std::string &buffer, int w, int h);
        SDL_Surface *rasterize_svg_image(NSVGimage *image, int w, int h);
        NSVGimage* parse_from_file(const char* filename, const char* units, float dpi);
        void delete_image(NSVGimage *image);
    };

    SDL_Surface *load_surface(std::string &file);
    SDL_Surface* create_shadow(SDL_Surface *in, const std::vector<BoxShadow> &box_shadows, int s_offset);
}
