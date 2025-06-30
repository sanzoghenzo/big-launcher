#pragma once

#include <string>
#include <SDL3/SDL.h>

struct TTF_Font;
namespace BL {
    class Font {
        private:
            TTF_Font *font = nullptr;
            SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };

        public:
            Font(const std::string &file, int height);
            ~Font() = default;
            SDL_Surface *render_text(const std::string &text, SDL_Rect *src_rect, SDL_Rect *dst_rect, int max_width);
    };
}
