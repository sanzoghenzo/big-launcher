#include "sidebar_entry.hpp"
#include "menu.hpp"
#include "text.hpp"

BL::SidebarEntry::SidebarEntry(std::string &&title, std::variant<BL::Menu*, std::string> &&value):
    BL::Drawable(),
    title(title),
    value(value)
{

}
BL::SidebarEntry::~SidebarEntry() = default;

void BL::SidebarEntry::render_surface(Font &font, int max_width)
{
    surface = font.render_text(title, nullptr, nullptr, max_width);
    pos.w = surface->w;
    pos.h = surface->h;
}
void BL::SidebarEntry::render_texture()
{
    texture = renderer->create_texture(*surface);
}
void BL::SidebarEntry::set_text_color(const SDL_Color &color)
{ 
    if (texture)
        texture->set_color_mod(color);
}