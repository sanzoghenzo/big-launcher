#include "renderer_sdl.hpp"
#include "logger.hpp"

void BL::TextureSDL::set_color_mod(const SDL_Color &color)
{
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);
}


BL::RendererSDL::RendererSDL(SDL_Window &window):
    Renderer(window),
    renderer(SDL_CreateRenderer(&window, nullptr))
{
    // Make sure the renderer supports the required format
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    const auto formats = reinterpret_cast<const SDL_PixelFormat*>(SDL_GetPointerProperty(props, SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, nullptr));
    if (!formats)
        BL::logger::critical("Could not detect pixel formats supported by renderer");
    int i = 0;
    for (i; formats[i] != SDL_PIXELFORMAT_UNKNOWN; i++) {
        if (formats[i] == SDL_PIXELFORMAT_ARGB8888)
            break;
    }
    if (formats[i] == SDL_PIXELFORMAT_UNKNOWN)
        BL::logger::critical("GPU does not support the required pixel format");

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
}

void BL::RendererSDL::set_draw_color(const SDL_Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void BL::RendererSDL::clear()
{
    SDL_RenderClear(renderer);
}

void BL::RendererSDL::present()
{
    SDL_RenderPresent(renderer);
}

BL::Texture* BL::RendererSDL::create_texture(SDL_Surface &surface)
{
    return new BL::TextureSDL(SDL_CreateTextureFromSurface(renderer, &surface));
}

BL::Texture* BL::RendererSDL::create_texture(SDL_Surface &surface, int w, int h)
{
    SDL_Texture *src_texture = SDL_CreateTextureFromSurface(renderer, &surface);
    SDL_Texture *dst_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetTextureBlendMode(dst_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, dst_texture);
    SDL_RenderTexture(renderer, src_texture, nullptr, nullptr);
    SDL_DestroyTexture(src_texture);
    return new BL::TextureSDL(dst_texture);
}

BL::Texture* BL::RendererSDL::create_texture(int w, int h)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
    return new BL::TextureSDL(texture);
}


void BL::RendererSDL::draw(Texture &texture)
{
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderTexture(renderer,
        const_cast<SDL_Texture*>(static_cast<BL::TextureSDL&>(texture).get_texture()),
        &texture.get_tex_coords(),
        &texture.get_pos()
    );
}

void BL::RendererSDL::composit_texture(const Texture &src, const Texture &dst, SDL_FRect *coords)
{
    SDL_SetRenderTarget(renderer, const_cast<SDL_Texture*>(static_cast<const BL::TextureSDL&>(dst).get_texture()));
    SDL_RenderTexture(renderer, const_cast<SDL_Texture*>(static_cast<const BL::TextureSDL&>(src).get_texture()), nullptr, coords);
    SDL_SetRenderTarget(renderer, nullptr);
}

void BL::RendererSDL::set_clip_rect(SDL_Rect &rect)
{
    SDL_SetRenderClipRect(renderer, &rect);
}

void BL::RendererSDL::disable_clip()
{
    SDL_SetRenderClipRect(renderer, nullptr);
}

void BL::RendererSDL::set_render_scale(float scale_w, float scale_h)
{
    SDL_SetRenderScale(renderer, scale_w, scale_h);
}

void BL::RendererSDL::set_logical_representation(int w, int h)
{
    SDL_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
}
