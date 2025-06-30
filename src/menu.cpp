#include <string>
#include <memory>

#include <SDL3/SDL.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "logger.hpp"
#include "external/nanosvg.h"

#include "menu.hpp"
#include "image.hpp"
#include "util.hpp"
#include "renderer.hpp"

namespace BL {
    void free_xml_char(xmlChar *c);
    using pXmlChar = std::unique_ptr<xmlChar, decltype(&free_xml_char)>;
    constexpr float CARD_ICON_MARGIN = 0.12f;
    constexpr float  MAX_CARD_ICON_MARGIN = 0.2f;
}

BL::MenuEntry::MenuEntry(const std::string &title, const std::string &command):
    BL::Drawable(),
    title(title),
    command(command),
    icon_margin(BL::CARD_ICON_MARGIN)
{}
BL::MenuEntry::~MenuEntry()
{
    BL::free_surface(icon_surface);
}

// Custom card
void BL::MenuEntry::set_card(const std::string &path)
{
    card_type = CardType::CUSTOM;
    this->path = path;
}

// Generated card, color background
void BL::MenuEntry::set_card(SDL_Color &background_color, const std::string &path)
{
    card_type = CardType::GENERATED;
    icon_path = path;
    this->background_color = background_color;
}

// Generated card, image background
void BL::MenuEntry::set_card(const std::string &background_path, const std::string &icon_path)
{
    card_type = CardType::GENERATED;
    path = background_path;
    this->icon_path = icon_path;
}

void BL::MenuEntry::set_margin(const char *value)
{
    std::string_view string = value;
    if (string.back() != '%')
        return;
    
    float percent = atof(std::string(string, 0, string.size() - 1).c_str()) / 100.f;
    if ((percent == 0.f && string != "0%") || percent < 0.f || percent > BL::MAX_CARD_ICON_MARGIN)
        return;
    icon_margin = percent;
}

bool BL::MenuEntry::render_surface(BL::SVGRasterizer &rasterizer, float w, float h, float shadow_offset)
{
    this->shadow_offset = shadow_offset;
    set_w(w);
    set_h(h);

    // Custom card
    if (card_type == BL::MenuEntry::CardType::CUSTOM) {
        surface = (path.ends_with(".svg")) 
                                ? rasterizer.rasterize_svg_from_file(path, w, h)
                                : BL::load_surface(path);
        if (!surface) {
            BL::logger::error("Failed to load card '{}'", path);
            return false;
        }
    }

    // Generated card
    else {
        if (!path.empty()) {
            surface = (path.ends_with(".svg")) 
                    ? rasterizer.rasterize_svg_from_file(path, w, h)
                    : BL::load_surface(path);
            if (!surface) {
                BL::logger::error("Failed to load card background '{}'", path);
                return false;
            }
        }

        // Color background
        if (!surface) {
            surface = SDL_CreateSurface(std::round(w), std::round(h), SDL_PIXELFORMAT_RGBA32);
            Uint32 color = SDL_MapSurfaceRGBA(surface, 
                                background_color.r, 
                                background_color.g, 
                                background_color.b, 
                                background_color.a
                            );
            SDL_FillSurfaceRect(surface, nullptr, color);
        }

        // Calculate aspect ratio, load surface if non-SVG
        float aspect_ratio, icon_w, icon_h;
        bool svg = icon_path.ends_with(".svg");
        NSVGimage *image = nullptr;
        if (svg) {
            image = rasterizer.parse_from_file(icon_path.c_str(), "px", 96.0f);
            if (!image) {
                BL::logger::error("Failed to load card icon '{}'", icon_path);
                return false;
            }
            icon_w = image->width;
            icon_h = image->height;
        }
        else {
            icon_surface = BL::load_surface(icon_path);
            if (!icon_surface) {
                BL::logger::error("Failed to load card icon '{}'", icon_path);
                return false;
            }
            icon_w = static_cast<float>(icon_surface->w);
            icon_h = static_cast<float>(icon_surface->h);
        }
        aspect_ratio = icon_w / icon_h;
        float target_w, target_h;

        // Calculate icon dimensions
        if (aspect_ratio >  BL::CARD_ASPECT_RATIO) {
            target_w = w  * (1.0f - 2.0f * icon_margin);
            target_h = ((target_w / icon_w)) * icon_h;
            icon_rect =  {
                std::round(icon_margin * w) + shadow_offset,
                std::round((h - target_h) / 2 + shadow_offset),
                std::round(target_w),
                std::round(target_h)
            };
        }
        else {
            target_h = h  * (1.0f - 2.0f * icon_margin);
            target_w = (target_h / icon_h) * icon_w;
            icon_rect = {
                std::round((w - target_w) / 2 + shadow_offset),
                std::round(icon_margin * h) + shadow_offset,
                std::round(target_w),
                std::round(target_h)
            };
        }
        if (svg) {
            icon_surface = rasterizer.rasterize_svg_image(image, icon_rect.w, icon_rect.h);
            if (!icon_surface) {
                BL::logger::error("Failed to load card icon '{}'", icon_path);
                return false;
            }
        }
    }

    return true;
}

void BL::MenuEntry::render_texture(Texture &shadow_texture, float shadow_offset, float card_w, float card_h)
{
    texture = renderer->create_texture(std::round(pos.w), std::round(pos.h));
    
    // Copy the shadow
    renderer->composit_texture(shadow_texture, *texture, nullptr);

    // Copy the background
    BL::Texture *background_texture = renderer->create_texture(*surface);
    SDL_FRect rect = {shadow_offset, shadow_offset, card_w, card_h};
    renderer->composit_texture(*background_texture, *texture, &rect);
    BL::free_surface(surface);
    surface = nullptr;
    delete background_texture;   

    // Copy the icon
    if (card_type == BL::MenuEntry::CardType::GENERATED) {
        BL::Texture *icon_texture = renderer->create_texture(*icon_surface);
        renderer->composit_texture(*icon_texture, *texture, &icon_rect);
        BL::free_surface(icon_surface);
        icon_surface = nullptr;
        delete icon_texture;           
    }
}

BL::Menu::Menu(const std::string &title, int nb_columns):
    BL::Object(),
    title(title),
    nb_columns(nb_columns)
{}
BL::Menu::~Menu() = default;

bool BL::Menu::parse(xmlNodePtr node)
{
    for (node = node->xmlChildrenNode; node != nullptr; node = node->next) {
        if (!xmlStrcmp(node->name, (const xmlChar*) "entry"))
            add_entry(node);
    }
    int num_entries = entry_list.size();
    if (!num_entries)
        return false;

    current_entry = entry_list.begin();
    max_columns = (num_entries < nb_columns) ? num_entries : nb_columns;
    total_rows = num_entries / max_columns;
    if (num_entries % max_columns)
        total_rows++;

    return true;
}

void BL::Menu::add_entry(xmlNodePtr node)
{
    BL::pXmlChar entry_title(xmlGetProp(node, (const xmlChar*) "title"), &BL::free_xml_char);
    if (!entry_title) {
        BL::logger::error("'menu' element in line {} is missing 'title' attribute", node->line);
        return;
    }
    
    xmlNodePtr cmd_node = nullptr;
    xmlNodePtr card_node = nullptr;

    // Get location of command and card elements
    for (xmlNodePtr current_node = node->xmlChildrenNode; current_node != nullptr; current_node = current_node->next) {
        if (!xmlStrcmp(current_node->name, (const xmlChar*) "command") && !cmd_node)
            cmd_node = current_node;
        else if (!xmlStrcmp(current_node->name, (const xmlChar*) "card") && !card_node)
            card_node = current_node;
    }

    if (!cmd_node) {
        BL::logger::error("Menu '{}': Entry '{}' is missing 'command' element", title, (const char*) entry_title.get());
        return;
    }
    if (!card_node) {
        BL::logger::error("Menu '{}': Entry '{}' is missing 'card' element", title, (const char*) entry_title.get());
        return;
    }

    // Parse command
    BL::pXmlChar command(xmlNodeGetContent(cmd_node), &BL::free_xml_char);
    if (!command) {
        BL::logger::error("Menu '{}', Entry '{}': element 'command' has no content", title, (const char*) entry_title.get());
        return;
    }
    auto entry = std::make_unique<BL::MenuEntry>((const char*) entry_title.get(), (const char*) command.get());

    // Parse card
    BL::pXmlChar content(xmlNodeGetContent(card_node), &BL::free_xml_char);
    unsigned long child_count = xmlChildElementCount(card_node);

    // Custom card
    if (content && !child_count)
        entry->set_card((const char*) content.get());

    // Generated card
    else if (child_count) {
        xmlNodePtr icon_node = nullptr;
        xmlNodePtr background_node = nullptr;

        // Get icon and background elements
        for (card_node = card_node->xmlChildrenNode; card_node != nullptr; card_node = card_node->next) {
            if (!xmlStrcmp(card_node->name, (const xmlChar*) "icon"))
                icon_node = card_node;
            else if (!xmlStrcmp(card_node->name, (const xmlChar*) "background"))
                background_node = card_node;
        }

        if (!icon_node || xmlChildElementCount(icon_node)) {
            BL::logger::error("Menu '{}', Entry '{}': generated card is missing 'icon' element",title, entry->get_title());
            return;
        }
        if (background_node && xmlChildElementCount(background_node))
            background_node = nullptr;

        // Get custom margin
        BL::pXmlChar margin(xmlGetProp(icon_node, (const xmlChar*) "margin"), &BL::free_xml_char);
        if (margin)
            entry->set_margin((const char*) margin.get());

        BL::pXmlChar icon(xmlNodeGetContent(icon_node), &BL::free_xml_char);
        BL::pXmlChar background((background_node) ? xmlNodeGetContent(background_node) : nullptr, &BL::free_xml_char);
        if (!icon) {
            BL::logger::error("Menu '{}', Entry '{}': 'icon' element in generated card has no content", title, entry->get_title());
            return;
        }

        SDL_Color color;
        bool is_color;
        if (!background) {
            color = {0xFF, 0xFF, 0xFF, 0xFF};
            is_color = true;
        }
        else
            is_color = BL::hex_to_color((char*) background.get(), color);

        if (is_color)
            entry->set_card(color, (const char*) icon.get());
        else
            entry->set_card((const char*) background.get(), (const char*) icon.get());
    }
    entry_list.emplace_back(std::move(*entry));
}

bool BL::Menu::render_surfaces(BL::SVGRasterizer &rasterizer, float w, float h, float shadow_offset)
{
    bool ret = true;
    for (MenuEntry &entry : entry_list) {
        if(!entry.render_surface(rasterizer, w, h, shadow_offset)) {
            entry.set_card_error(true);
            ret = false;
        }
    }
    return ret;
}

void BL::Menu::render_card_textures(Texture &card_shadow_texture, float shadow_offset, float card_w, float card_h)
{
    for (MenuEntry &entry : entry_list) {
        entry.set_renderer(*renderer);
        if (!entry.get_card_error())
            entry.render_texture(card_shadow_texture, shadow_offset, card_w, card_h);
    }
}

void BL::Menu::draw()
{
    for (MenuEntry &entry : entry_list)
        if (entry.get_card_error()) {
            error_texture->update_pos(entry.get_pos());
            renderer->draw(*error_texture);
        }
        else
            entry.draw();
}

void BL::Menu::print_entries()
{
    for (MenuEntry &entry : entry_list) {
        BL::logger::debug("Entry {}:", &entry - &entry_list[0]);
        BL::logger::debug("Title: {}", entry.get_title());
        BL::logger::debug("Command: {}", entry.get_command());
    }
}

void BL::Menu::set_x(float x)
{
    float x_start = x;
    int col = 0;
    for (MenuEntry &entry : entry_list) {
        entry.set_x(x);
        col++;
        if (col == nb_columns) {
            x = x_start;
            col = 0;
        }
        else
            x += x_advance;
    }
}

void BL::Menu::set_y(float y)
{
    float y_start = y;
    int col = 0;
    for (MenuEntry &entry : entry_list) {
        entry.set_y(y);
        col++;
        if (col == nb_columns) {
            y += y_advance;
            col = 0;
        }
    }
}

void BL::Menu::inc_x(float x)
{
    for (MenuEntry &entry : entry_list)
        entry.inc_x(x);

}

void BL::Menu::dec_x(float x)
{
    for (MenuEntry &entry : entry_list)
        entry.dec_x(x);
}

void BL::Menu::inc_y(float y)
{
    for (MenuEntry &entry : entry_list)
        entry.inc_y(y);
}

void BL::Menu::dec_y(float y)
{
    for (MenuEntry &entry : entry_list)
        entry.dec_y(y);
}
