#include <cmath>
#include <algorithm>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "logger.hpp"
#include <lconfig.h>
#include "image.hpp"
#include "util.hpp"
#define NANOSVG_IMPLEMENTATION
#include "external/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "external/nanosvgrast.h"
#include "external/fast_gaussian_blur_template.h"

SDL_Surface* BL::load_surface(std::string &file)
{
    SDL_Surface *img = nullptr;
    SDL_Surface *out = nullptr;
    img = IMG_Load(file.c_str());
    if (!img) {
        BL::logger::error("Could not load image from {} (SDL Error: {})", file, SDL_GetError());
        return out;
    }

    // Convert the loaded surface if different pixel format
    if (img->format != SDL_PIXELFORMAT_RGBA32) {
        out = SDL_CreateSurface(img->w, img->h, SDL_PIXELFORMAT_RGBA32);
        Uint32 color = SDL_MapSurfaceRGBA(out, 0xFF, 0xFF, 0xFF, 0);
        SDL_FillSurfaceRect(out, nullptr, color);
        SDL_BlitSurface(img, nullptr, out, nullptr);
        BL::free_surface(img);
    }
    else
        out = img;

    return out;
}

BL::SVGRasterizer::SVGRasterizer()
{
    rasterizer = nsvgCreateRasterizer();
    if (!rasterizer)
        BL::logger::critical("Could not initialize SVG rasterizer");
}

BL::SVGRasterizer::~SVGRasterizer()
{
    if (rasterizer)
        nsvgDeleteRasterizer(rasterizer);
}


SDL_Surface* BL::SVGRasterizer::rasterize_svg_from_file(const std::string &file, int w, int h)
{
    NSVGimage *image = nsvgParseFromFile((char*) file.c_str(), "px", 96.0f);
    if (!image) {
        BL::logger::error("Could not load SVG");
        return nullptr;
    }
    return rasterize_svg_image(image, w, h);
}

// A function to rasterize an SVG from an existing text buffer
SDL_Surface* BL::SVGRasterizer::rasterize_svg(const std::string &buffer, int w, int h)
{
    NSVGimage *image = nsvgParse((char*) buffer.c_str(), "px", 96.0f);
    if (!image) {
        BL::logger::error("Could not parse SVG");
        return nullptr;
    }
    return rasterize_svg_image(image, w, h);
}

SDL_Surface* BL::SVGRasterizer::rasterize_svg_image(NSVGimage *image, int w, int h)
{
    unsigned char *pixel_buffer = nullptr;
    int width, height, pitch;
    float scale;

    // Calculate scaling and dimensions
    if (w == -1 && h == -1) {
        scale = 1.0f;
        width = static_cast<int>(image->width);
        height = static_cast<int>(image->height);
    }
    else if (w == -1 && h != -1) {
        scale = static_cast<float>(h) / (float) image->height;
        width = static_cast<int>(ceil(static_cast<double>(image->width * scale)));
        height = h;
    }
    else if (w != -1 && h == -1) {
        scale = static_cast<float>(w) / image->width;
        width = w;
        height = static_cast<int>(ceil(static_cast<double>(image->height * scale)));
    }
    else {
        scale = static_cast<float>(w) / image->width;
        width = w;
        height = h;
    }
    
    // Allocate memory
    pitch = 4*width;
    pixel_buffer = reinterpret_cast<unsigned char*>(malloc(4*width*height));
    if (!pixel_buffer) {
        BL::logger::error("Could not alloc SVG pixel buffer");
        return nullptr;
    }

    // Rasterize image
    nsvgRasterize(rasterizer, image, 0, 0, scale, pixel_buffer, width, height, pitch);
    SDL_Surface *surface = SDL_CreateSurfaceFrom(
                               width,
                               height,
                               SDL_PIXELFORMAT_RGBA32,
                               pixel_buffer,
                               pitch
                           );
    nsvgDelete(image);
    return surface;
}

NSVGimage* BL::SVGRasterizer::parse_from_file(const char* filename, const char* units, float dpi)
{
    return nsvgParseFromFile(filename, units, dpi);
}

void BL::SVGRasterizer::delete_image(NSVGimage *image)
{
    if (image)
        nsvgDelete(image);
}


SDL_Surface* BL::create_shadow(SDL_Surface *in, const std::vector<BoxShadow> &box_shadows, int s_offset)
{
    float max_radius = 0.0f;
    std::for_each(box_shadows.begin(), 
        box_shadows.end(), 
        [&](const BoxShadow &bs){if(bs.radius > max_radius) max_radius = bs.radius;}
    );
    int padding = 2 * static_cast<int>(ceil(max_radius));

    SDL_Color mod;
    SDL_GetSurfaceColorMod(in, &mod.r, &mod.g, &mod.b);
    SDL_GetSurfaceAlphaMod(in, &mod.a);
    SDL_SetSurfaceColorMod(in, 0, 0, 0);

    // Set up shadow
    SDL_Surface *shadow = SDL_CreateSurface(
                              in->w + 2*s_offset, 
                              in->h + 2*s_offset, 
                              SDL_PIXELFORMAT_RGBA32
                          );
    Uint32 color = SDL_MapSurfaceRGBA(shadow, 0, 0, 0, 0);
    SDL_FillSurfaceRect(shadow, nullptr, color);

    // Set up alpha mask
    SDL_Surface *alpha_mask = SDL_CreateSurface(
                                  in->w + 2*(padding + s_offset), 
                                  in->h + 2*(padding + s_offset), 
                                  SDL_PIXELFORMAT_RGBA32
                              );
    SDL_Rect alpha_mask_rect = {padding + s_offset, padding + s_offset, in->w, in->h};
    
    Uint8 *in_pixels;
    Uint8 *out_pixels;
    Uint8 *out_pixels0; // fast guassian blur swaps the address of the in/out pointers, need to rememeber it
    SDL_Surface *tmp;
    SDL_Rect src_rect;
    SDL_Rect dst_rect;
    int w, h;
    for (const BoxShadow &bs : box_shadows) {

        // Make alpha mask
        SDL_FillSurfaceRect(alpha_mask, nullptr, color);
        SDL_SetSurfaceAlphaMod(in, bs.alpha);
        SDL_BlitSurface(in, nullptr, alpha_mask, &alpha_mask_rect);

        // Blur alpha mask
        in_pixels = static_cast<Uint8*>(alpha_mask->pixels);
        out_pixels = static_cast<Uint8*>(malloc(alpha_mask->w*alpha_mask->h*4));
        out_pixels0 = out_pixels;
        fast_gaussian_blur<Uint8>(in_pixels, out_pixels, alpha_mask->w, alpha_mask->h, 4, bs.radius);
        tmp = SDL_CreateSurfaceFrom(
                  alpha_mask->w,
                  alpha_mask->h,
                  SDL_PIXELFORMAT_RGBA32,
                  out_pixels,
                  alpha_mask->w*4
              );

        // Composit onto shadow surface
        w = alpha_mask->w - 2*padding - abs(bs.x_offset);
        h = alpha_mask->w - 2*padding - abs(bs.y_offset);
        src_rect = {
            (bs.x_offset >= 0) ? padding : padding + static_cast<int>(bs.x_offset), 
            (bs.y_offset >= 0) ? padding : padding + static_cast<int>(bs.y_offset), 
            w,
            h
        };
        dst_rect = {
            (bs.x_offset > 0) ? static_cast<int>(bs.x_offset) : 0,
            (bs.y_offset > 0) ? static_cast<int>(bs.y_offset) : 0,
            w,
            h
        };
        SDL_BlitSurface(tmp, &src_rect, shadow, &dst_rect);
        free(out_pixels0);
        SDL_DestroySurface(tmp);
    }

    BL::free_surface(alpha_mask);

    SDL_SetSurfaceColorMod(in, mod.r, mod.g, mod.b);
    SDL_SetSurfaceAlphaMod(in, mod.a);

    return shadow;
}
