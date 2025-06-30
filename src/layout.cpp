#include <string>
#include <set>
#include <memory>
#include <array>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "logger.hpp"
#include <SDL3/SDL.h>
#include <lconfig.h>
#include "config.hpp"
#include "layout.hpp"
#include "image.hpp"
#include "main.hpp"
#include "menu.hpp"
#include "menu_highlight.hpp"
#include "renderer.hpp"
#include "screensaver.hpp"
#include "sidebar_entry.hpp"
#include "sidebar_highlight.hpp"
#include "text.hpp"
#include "util.hpp"

#define ERROR_FORMAT "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> <svg version=\"1.1\" id=\"Ebene_1\" x=\"0px\" y=\"0px\" width=\"140.50626\" height=\"140.50626\" viewBox=\"0 0 140.50625 140.50626\" xml:space=\"preserve\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\"><defs id=\"defs17\" /> <g id=\"layer1\" transform=\"matrix(1.0014475,0,0,0.99627733,-130.32833,-78.42333)\" style=\"fill:#ffffff\" /><g id=\"g4\" transform=\"matrix(0.32150107,0,0,0.32150107,-27.692492,-70.907371)\"> <path style=\"fill:#ffffff\" d=\"m 326.039,513.568 h -69.557 v -9.441 c 0,-10.531 2.12,-19.876 6.358,-28.034 4.239,-8.156 13.165,-18.527 26.783,-31.117 l 12.33,-11.176 c 7.322,-6.678 12.684,-12.973 16.09,-18.882 3.4,-5.907 5.105,-11.817 5.105,-17.727 0,-8.99 -3.084,-16.022 -9.248,-21.098 -6.166,-5.073 -14.773,-7.611 -25.819,-7.611 -10.405,0 -21.646,2.152 -33.719,6.455 -12.075,4.305 -24.663,10.693 -37.765,19.171 v -60.5 c 15.541,-5.395 29.735,-9.375 42.582,-11.946 12.843,-2.568 25.241,-3.854 37.186,-3.854 31.342,0 55.232,6.392 71.678,19.171 16.439,12.783 24.662,31.439 24.662,55.973 0,12.591 -2.506,23.862 -7.516,33.815 -5.008,9.956 -13.553,20.649 -25.625,32.08 l -12.332,10.983 c -8.736,7.966 -14.451,14.354 -17.148,19.171 -2.697,4.817 -4.045,10.115 -4.045,15.896 z m -69.557,28.517 h 69.557 v 68.593 h -69.557 z\" id=\"path2\" /> </g> <circle style=\"fill:#f44336;stroke-width:0.321501\" cx=\"70.253128\" cy=\"70.253128\" id=\"circle6\" r=\"70.253128\" /> <g id=\"g12\" transform=\"matrix(0.32150107,0,0,0.32150107,-26.147362,-70.907371)\"> <rect x=\"267.16199\" y=\"307.978\" transform=\"matrix(0.7071,-0.7071,0.7071,0.7071,-222.6202,340.6915)\" style=\"fill:#ffffff\" width=\"65.544998\" height=\"262.17999\" id=\"rect8\" /> <rect x=\"266.98801\" y=\"308.15302\" transform=\"matrix(0.7071,0.7071,-0.7071,0.7071,398.3889,-83.3116)\" style=\"fill:#ffffff\" width=\"65.543999\" height=\"262.17899\" id=\"rect10\" /> </g> <g id=\"g179\" transform=\"matrix(0.32150107,0,0,0.32150107,-27.692492,-70.907371)\"> <path style=\"fill:#ffffff\" d=\"m 326.039,513.568 h -69.557 v -9.441 c 0,-10.531 2.12,-19.876 6.358,-28.034 4.239,-8.156 13.165,-18.527 26.783,-31.117 l 12.33,-11.176 c 7.322,-6.678 12.684,-12.973 16.09,-18.882 3.4,-5.907 5.105,-11.817 5.105,-17.727 0,-8.99 -3.084,-16.022 -9.248,-21.098 -6.166,-5.073 -14.773,-7.611 -25.819,-7.611 -10.405,0 -21.646,2.152 -33.719,6.455 -12.075,4.305 -24.663,10.693 -37.765,19.171 v -60.5 c 15.541,-5.395 29.735,-9.375 42.582,-11.946 12.843,-2.568 25.241,-3.854 37.186,-3.854 31.342,0 55.232,6.392 71.678,19.171 16.439,12.783 24.662,31.439 24.662,55.973 0,12.591 -2.506,23.862 -7.516,33.815 -5.008,9.956 -13.553,20.649 -25.625,32.08 l -12.332,10.983 c -8.736,7.966 -14.451,14.354 -17.148,19.171 -2.697,4.817 -4.045,10.115 -4.045,15.896 z m -69.557,28.517 h 69.557 v 68.593 h -69.557 z\" id=\"path177\" /> </g><circle style=\"fill:#f44336;stroke-width:0.321501\" cx=\"70.253128\" cy=\"70.253128\" id=\"circle181\" r=\"70.253128\" /><g id=\"g187\" transform=\"matrix(0.32150107,0,0,0.32150107,-26.147362,-70.907371)\"> <rect x=\"267.16199\" y=\"307.978\" transform=\"matrix(0.7071,-0.7071,0.7071,0.7071,-222.6202,340.6915)\" style=\"fill:#ffffff\" width=\"65.544998\" height=\"262.17999\" id=\"rect183\" /> <rect x=\"266.98801\" y=\"308.15302\" transform=\"matrix(0.7071,0.7071,-0.7071,0.7071,398.3889,-83.3116)\" style=\"fill:#ffffff\" width=\"65.543999\" height=\"262.17899\" id=\"rect185\" /> </g></svg>"

extern "C" void libxml2_error_handler(void *ctx, const char *msg, ...);
extern BL::Config config;

namespace BL {
    void free_xml_char(xmlChar *c)
    {
        xmlFree((void*) c);
    }
    using pXmlChar = std::unique_ptr<xmlChar, decltype(&free_xml_char)>;
    using pXmlDoc = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;

    constexpr float SIDEBAR_SHIFT_TIME = 200.0f;
    constexpr float ROW_SHIFT_TIME = 120.0f;
    constexpr float HIGHLIGHT_SHIFT_TIME = 100.0f;
    constexpr float ENTRY_PRESS_TIME = 100;
    constexpr float ENTRY_SHRINK_DISTANCE = 0.04f;
    constexpr int COLUMNS = 3;
    constexpr float TOP_MARGIN = 0.2f;
    constexpr float BOTTOM_MARGIN = 1.0f;
    constexpr float SIDEBAR_HIGHLIGHT_LEFT = 0.08f;
    constexpr float SIDEBAR_HIGHLIGHT_RIGHT = 0.30f;
    constexpr float SIDEBAR_HIGHLIGHT_WIDTH = SIDEBAR_HIGHLIGHT_RIGHT - SIDEBAR_HIGHLIGHT_LEFT;
    constexpr float SIDEBAR_HIGHLIGHT_HEIGHT = 0.06f;
    constexpr float SIDEBAR_CORNER_RADIUS = 0.011f;
    constexpr float SIDEBAR_FONT_SIZE = 0.55f;
    constexpr float SIDEBAR_Y_ADVANCE = 0.068f;
    constexpr float SIDEBAR_TEXT_MARGIN = 0.07f;
    constexpr float CARD_LEFT_MARGIN = 0.42f;
    constexpr float CARD_RIGHT_MARGIN = 0.92f;
    constexpr float CARD_SPACING = 0.011f;
    constexpr float CARD_WIDTH = CARD_RIGHT_MARGIN - CARD_LEFT_MARGIN;
    constexpr float ERROR_ICON_MARGIN = 0.35f;
}

// Wrapper for libxml2 error messages
void libxml2_error_handler(void *ctx, const char *msg, ...)
{
    static char error[512];
    static int error_length = 0;
    if (!error_length)
        memset(error, '\0', sizeof(error));

    char message[512];
    va_list args;
    va_start(args, msg);
    vsnprintf(message, sizeof(message), msg, args);
    va_end(args);

    int message_length = strlen(message);
    size_t bytes = (error_length + message_length < sizeof(error)) ? message_length : sizeof(error) - error_length - 1;
    strncat(error, message, sizeof(error) - error_length - 1);
    error_length += bytes;

    if (error[error_length - 1] == '\n' || error_length == (sizeof(error) - 1)) {
        error[error_length - 1] = '\0';
        BL::logger::error("{}", error);
        error_length = 0;
    }
}

void BL::Layout::render_error_texture()
{
    error_texture = renderer->create_texture(card_w + 2*card_shadow_offset, card_h + 2*card_shadow_offset);
    renderer->composit_texture(*card_shadow_texture, *error_texture, nullptr);

    SDL_FRect bg_rect = {card_shadow_offset, card_shadow_offset, card_w, card_h};
    BL::Texture *error_bg_texture = renderer->create_texture(*error_bg);
    BL::free_surface(error_bg);
    error_bg = nullptr;

    renderer->composit_texture(*error_bg_texture, *error_texture, &bg_rect);
    delete error_bg_texture;

    BL::Texture *error_icon_texture = renderer->create_texture(*error_icon);
    BL::free_surface(error_icon);
    error_icon = nullptr;

    renderer->composit_texture(*error_icon_texture, *error_texture, &error_icon_rect);
    delete error_icon_texture;

    // Assign texture to all menus 
    for (BL::Menu &menu : menus)
        menu.set_error_texture(*error_texture);
}

void BL::Layout::parse(const std::string &file)
{
    BL::logger::debug("Parsing layout file '{}'", file);
    xmlNodePtr node;
    
    xmlSetGenericErrorFunc(nullptr, libxml2_error_handler);
    BL::pXmlDoc doc(xmlParseFile(file.c_str()), &xmlFreeDoc);
    if (!doc)
        BL::logger::critical("Failed to parse layout file");
    node = xmlDocGetRootElement(doc.get());
    if (!node)
        BL::logger::critical("Could not get root element of layout file");

    if (xmlStrcmp(node->name, (const xmlChar*) "layout"))
        BL::logger::critical("Root element of layout file is not <layout>");

    for (node = node->xmlChildrenNode; node != nullptr; node = node->next) {

        // Menu detected
        if (!xmlStrcmp(node->name, (const xmlChar*) "menu")) {
            BL::pXmlChar title(xmlGetProp(node, (const xmlChar*) "title"), &BL::free_xml_char);
            if (!title)
                BL::logger::error("In layout file, <menu> element in line {} has no 'title' attribute", node->line);
            else {
                auto menu = std::make_unique<BL::Menu>((const char*) title.get(), BL::COLUMNS);
                if (menu->parse(node)) {
                    menus.emplace_back(std::move(*menu));
                    sidebar_entries.emplace_back((const char*) title.get(), nullptr);
                }
            }
        }

        // Command detected
        else if (!xmlStrcmp(node->name, (const xmlChar*) "command")) {
            BL::pXmlChar title(xmlGetProp(node, (const xmlChar*) "title"), &BL::free_xml_char);
            if (title) {
                BL::pXmlChar cmd(xmlNodeGetContent(node), &BL::free_xml_char);
                if (cmd && !xmlChildElementCount(node))
                    sidebar_entries.emplace_back((const char*) title.get(), (const char*) cmd.get());
            }
        }
    }
    for (int i = 0; SidebarEntry &entry : sidebar_entries) {
        if (!entry.get_menu() && !entry.get_command())
            entry.set_menu(&menus[i++]);
    }
    num_sidebar_entries = sidebar_entries.size();
    current_entry = sidebar_entries.begin();
    current_menu = current_entry->get_menu();
    BL::logger::debug("Successfully parsed layout file");
}

BL::Layout::Press::Press(MenuEntry &entry):
    entry(&entry),
    original_rect({entry.get_x(), entry.get_y(), entry.get_w(), entry.get_h()}),
    total(std::round(original_rect.w * BL::ENTRY_SHRINK_DISTANCE)),
    current(0.f),
    velocity(2.f * total / static_cast<float>(BL::ENTRY_PRESS_TIME)),
    direction(BL::Layout::Direction::RIGHT),
    aspect_ratio(original_rect.w / original_rect.h),
    ticks(SDL_GetTicks())
{}

void BL::Layout::load_background()
{
    if (!config.background_image_path.empty()) {
        background_surface = (config.background_image_path.ends_with(".svg")) 
                             ? rasterizer->rasterize_svg_from_file(config.background_image_path, screen_width, screen_height) 
                             : BL::load_surface(config.background_image_path);
    }
}

void BL::Layout::load_sidebar()
{
    // Sidebar highlight geometry calculations and rendering
    float sidebar_width = std::round(f_screen_width * BL::SIDEBAR_HIGHLIGHT_WIDTH);
    float sidebar_height = std::round(f_screen_height * BL::SIDEBAR_HIGHLIGHT_HEIGHT);
    int sidebar_cx = static_cast<int>(std::round(sidebar_width * BL::SIDEBAR_CORNER_RADIUS));
    int sidebar_font_size = static_cast<int>(std::round(sidebar_height * BL::SIDEBAR_FONT_SIZE));
    sidebar_y_advance = std::round(f_screen_height * BL::SIDEBAR_Y_ADVANCE);

    sidebar_highlight = new BL::SidebarHighlight();
    sidebar_highlight->render_surface(*rasterizer, sidebar_width, sidebar_height, sidebar_cx);
    sidebar_highlight->set_x(std::round(f_screen_width * BL::SIDEBAR_HIGHLIGHT_LEFT));
    sidebar_highlight->set_y(y_min);
    sidebar_highlight_clip = {
        0, 
        static_cast<int>(std::round(y_min - sidebar_highlight->get_shadow_offset())),
        screen_width, 
        screen_height - static_cast<int>(std::round(y_min -sidebar_highlight->get_shadow_offset()))
    };

    // Find and load sidebar font
    BL::Font sidebar_font(SIDEBAR_FONT, sidebar_font_size);

    // Sidebar entry text geometry calculations and rendering
    float sidebar_text_margin = std::round(sidebar_width * BL::SIDEBAR_TEXT_MARGIN);
    float sidebar_text_x = std::round(sidebar_highlight->get_x() + sidebar_text_margin);
    float max_sidebar_text_width = std::round(sidebar_highlight->get_w() - 2 * sidebar_text_margin);
    float y = std::round(sidebar_highlight->get_y() + sidebar_highlight->get_h() / 2);
    for (int i = 0; SidebarEntry &entry : sidebar_entries) {
        entry.render_surface(sidebar_font, max_sidebar_text_width);
        entry.set_x(sidebar_text_x);
        entry.set_y(std::round(y - entry.get_h() / 2.f));
        if (max_sidebar_entries == -1 && (entry.get_y() + entry.get_h() > y_max))
            max_sidebar_entries = i - 1;
        y += sidebar_y_advance;
        i++;
    }
    sidebar_text_clip = {
        0, 
        static_cast<int>(std::round(y_min)),
        screen_width,
        static_cast<int>(std::round(y_max - y_min))
    };
}

void BL::Layout::load_menus()
{
    // Menu card geometry calculations
    card_x0 = std::round(f_screen_width * BL::CARD_LEFT_MARGIN);
    card_y0 = y_min;
    card_spacing = std::round(f_screen_width * BL::CARD_SPACING);
    card_w = ((std::round(f_screen_width * (BL::CARD_WIDTH))) - (BL::COLUMNS - 1)*card_spacing) / BL::COLUMNS;
    card_h = std::round(card_w / BL::CARD_ASPECT_RATIO);

    // Render card shadow
#ifdef __unix__
    constexpr
#endif
    Uint8 alpha = static_cast<Uint8>(std::round((255.f * BL::SHADOW_ALPHA)));
    float max_blur = BL::SHADOW_BLUR_SLOPE*card_h + BL::SHADOW_BLUR_INTERCEPT;
    float max_y_offset = BL::SHADOW_OFFSET_SLOPE*card_h + BL::SHADOW_OFFSET_INTERCEPT;
    std::vector<BoxShadow> box_shadows = {
        {0, std::round(max_y_offset / 2.f), std::round(max_blur / 2.f), alpha},
        {0, std::round(max_y_offset),     std::round(max_blur),        alpha}
    };
    card_shadow_offset = std::round(max_blur * 2.f);

    SDL_Surface *shadow_box = SDL_CreateSurface(card_w, card_h, SDL_PIXELFORMAT_RGBA32);
    Uint32 white = SDL_MapSurfaceRGBA(shadow_box, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_FillSurfaceRect(shadow_box, nullptr, white);
    card_shadow = BL::create_shadow(shadow_box, box_shadows, card_shadow_offset);
    BL::free_surface(shadow_box);

    // Load cards
    float card_x_advance = card_w + card_spacing;
    card_y_advance = card_h + card_spacing;
    max_rows = static_cast<int>(std::floor((y_max - y_min) / card_y_advance)); // max number of rows that can fit on the screen at once
    y_leftover = y_max - (y_min + static_cast<float>(max_rows) * card_h + static_cast<float>(max_rows - 1) * card_spacing);
    for (BL::Menu &menu : menus)
        card_error |= !menu.render_surfaces(*rasterizer, card_w, card_h, card_shadow_offset);
    if (card_error)
        render_error_surface();

    // Set positions
    float y = card_y0;
    menu_objects.reserve(menus.size());
    for (BL::SidebarEntry &entry : sidebar_entries) {
        if (BL::Menu *menu = entry.get_menu(); menu) {
            menu->set_x_advance(card_x_advance);
            menu->set_y_advance(card_y_advance);
            menu->set_x(card_x0);
            menu->set_y(y);
            y += std::max(menu->get_h() + y_leftover, f_screen_height);
            menu_objects.push_back(menu);
        }
        else
            y += f_screen_height;
    }

    menu_clip = {
        0,
        static_cast<int>(std::round(y_min - card_spacing / 2.f)),
        screen_width,
        screen_height - static_cast<int>(std::round(y_min - card_spacing / 2.f))
    };

}

void BL::Layout::load_menu_highlight()
{
    int t = static_cast<int>(std::round(card_spacing * HIGHLIGHT_THICKNESS));
    float inner_spacing = static_cast<int>(std::round(card_spacing * HIGHLIGHT_INNER_SPACING));
    highlight_x0 = card_x0 - (inner_spacing + t);
    highlight_y0 = card_y0 - (inner_spacing + t);
    int padding = 2*(inner_spacing + t);
    int highlight_w = static_cast<int>(card_w) + padding;
    int highlight_h = static_cast<int>(card_h) + padding; 

    menu_highlight = new BL::MenuHighlight();
    menu_highlight->render_surface(*rasterizer,
        highlight_w, 
        highlight_h, 
        t
    );
    menu_highlight->set_x(highlight_x0);
    menu_highlight->set_y(highlight_y0);
    highlight_x_advance = card_w + card_spacing;
    highlight_y_advance = card_h + card_spacing;
}

void BL::Layout::load_surfaces()
{
    BL::logger::debug("Rendering surfaces...");
    f_screen_width = static_cast<float>(screen_width);
    f_screen_height = static_cast<float>(screen_height);
    y_min = std::round(f_screen_height * BL::TOP_MARGIN);
    y_max = std::round(f_screen_height * BL::BOTTOM_MARGIN);

    load_background();
    load_sidebar();
    load_menus();
    load_menu_highlight();
 
    if (config.screensaver_enabled) {
        screensaver = new BL::Screensaver(screen_width, screen_height, launcher);
        screensaver->render_surface();
    }

    BL::logger::debug("Successfully rendered surfaces");
}

void BL::Layout::render_error_surface()
{
    if (error_bg || error_icon)
        return;
    
    error_bg = SDL_CreateSurface(static_cast<int>(card_w), static_cast<int>(card_h), SDL_PIXELFORMAT_RGBA32);
    Uint32 color = SDL_MapSurfaceRGBA(error_bg, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_FillSurfaceRect(error_bg, nullptr, color);

    // Geometry
    float target_h = std::round(card_h  * (1.0f - 2.0f * BL::ERROR_ICON_MARGIN));
    float target_w = target_h;
    error_icon_rect = {
        std::round((card_w - target_w) / 2.f + card_shadow_offset),
        std::round(BL::ERROR_ICON_MARGIN * card_h + card_shadow_offset),
        std::round(target_w),
        std::round(target_h)
    };
    error_icon = rasterizer->rasterize_svg(std::string(ERROR_FORMAT), target_w, target_h);
}

void BL::Layout::load_textures(BL::Renderer &renderer)
{
    this->renderer = &renderer;
    BL::logger::debug("Rendering textures...");

    // Background texture
    if (background_surface) {
        background_texture = renderer.create_texture(*background_surface, screen_width, screen_height);
        BL::free_surface(background_surface);
    }
    sidebar_highlight->set_renderer(renderer);
    sidebar_highlight->render_texture();

    // Render sidebar texts
    card_shadow_texture = renderer.create_texture(*card_shadow);
    BL::free_surface(card_shadow);
    card_shadow = nullptr;
    for (BL::SidebarEntry &entry : sidebar_entries) {
        entry.set_renderer(renderer);
        entry.render_texture();
        entry.set_text_color(&entry == &*current_entry ? config.sidebar_text_color_highlighted : config.sidebar_text_color);
    }

    // Render application cards
    for (BL::Menu &menu : menus) {
        menu.set_renderer(renderer);
        menu.render_card_textures(*card_shadow_texture, card_shadow_offset, card_w, card_h);
    }
    if (card_error)
        render_error_texture();
    delete card_shadow_texture;
    card_shadow_texture = nullptr;

    // Render menu highlight
    menu_highlight->set_renderer(renderer);
    menu_highlight->render_texture();

    // Render screensaver
    if (screensaver) {
        screensaver->set_renderer(renderer);
        screensaver->render_texture();
    }
    BL::logger::debug("Sucessfully rendered textures");
}

void BL::Layout::move_up()
{
    if (selection_mode == SelectionMode::SIDEBAR) {
        if (!sidebar_pos)
            return;
        if (sidebar_shift_count && sidebar_pos == sidebar_shift_count){
            add_shift(Shift::Type::SIDEBAR, Direction::DOWN, sidebar_y_advance, BL::SIDEBAR_SHIFT_TIME, {sidebar_highlight});
            sidebar_shift_count--;
        }

        // Shift menus if necessary
        if (!menu_objects.empty()) {
            float shift_amount;
            BL::Menu *next_menu = (current_entry - 1)->get_menu();
            shift_amount = next_menu ? card_y0 - next_menu->get_y() : f_screen_height;
            add_shift(
                Shift::Type::MENU,
                Direction::DOWN,
                shift_amount,
                BL::SIDEBAR_SHIFT_TIME,
                menu_objects,
                next_menu ? Shift::Method::ABS : Shift::Method::REL
            );
        }

        // Adjust texture color
        current_entry->set_text_color(config.sidebar_text_color);
        current_entry--;
        current_menu = current_entry->get_menu();
        current_entry->set_text_color(config.sidebar_text_color_highlighted);
        sidebar_highlight->dec_y(sidebar_y_advance);
        sidebar_pos--;
        launcher.play_click();
    }

    else if (selection_mode == SelectionMode::MENU && current_menu->get_row()) {
        if (current_menu->get_row() > 2) {
            current_menu->dec_shift_count();
            add_shift(Shift::Type::MENU, Direction::DOWN, (card_y0 - current_menu->get_shift_count() * card_y_advance) - current_menu->get_y(), BL::ROW_SHIFT_TIME, menu_objects, Shift::Method::ABS);
        }
        else
            add_shift(Shift::Type::HIGHLIGHT, Direction::UP, highlight_y_advance, BL::HIGHLIGHT_SHIFT_TIME, {menu_highlight});

        current_menu->dec_row();
        launcher.play_click();
    }
}

void BL::Layout::move_down()
{
    if (selection_mode == SelectionMode::SIDEBAR) {
        if ((sidebar_pos < (num_sidebar_entries - 1))) {

            // Shift menus if necessary
            if (!menu_objects.empty()) {
                float shift_amount;
                BL::Menu *next_menu = (current_entry + 1)->get_menu();
                shift_amount = next_menu ? next_menu->get_y() - card_y0 : f_screen_height;
                add_shift(
                    Shift::Type::MENU,
                    Direction::UP,
                    shift_amount,
                    BL::SIDEBAR_SHIFT_TIME,
                    menu_objects,
                    next_menu ? Shift::Method::ABS : Shift::Method::REL
                );
            }

            // Adjust text color
            current_entry->set_text_color(config.sidebar_text_color);
            current_entry++;
            current_menu = current_entry->get_menu();
            current_entry->set_text_color(config.sidebar_text_color_highlighted);
            sidebar_highlight->inc_y(sidebar_y_advance);
            sidebar_pos++;
            launcher.play_click();
        }

        // Shift sidebar highlight
        if (max_sidebar_entries != -1 && sidebar_pos < (num_sidebar_entries - max_sidebar_entries)) {
            add_shift(Shift::Type::SIDEBAR, Direction::UP, sidebar_y_advance, BL::SIDEBAR_SHIFT_TIME, {sidebar_highlight});
            sidebar_shift_count++;
        }
    }

    // We are in menu mode
    else if (selection_mode == SelectionMode::MENU) {
        if (current_menu->get_row() * BL::COLUMNS + current_menu->get_column() + BL::COLUMNS < current_menu->num_entries()) {

            if ((current_menu->get_row() < (current_menu->get_total_rows() - 1) && current_menu->get_row() >= (max_rows - 1))) {
                current_menu->inc_shift_count();
                add_shift(Shift::Type::MENU, Direction::UP, current_menu->get_y() - (card_y0 - current_menu->get_shift_count() * card_y_advance), BL::ROW_SHIFT_TIME, menu_objects, Shift::Method::ABS);
            }
            else
                add_shift(Shift::Type::HIGHLIGHT, Direction::DOWN, highlight_y_advance, BL::HIGHLIGHT_SHIFT_TIME, {menu_highlight});

            current_menu->inc_row();
            launcher.play_click();
        }
    }
}


void BL::Layout::move_left()
{
    if (selection_mode == SelectionMode::MENU) {
        if (current_menu->get_column() == 0) {
            if (shift_queue.empty()) {
                selection_mode = SelectionMode::SIDEBAR;
                current_entry->set_text_color(config.sidebar_text_color_highlighted);
                current_menu->reset_row();
                current_menu->reset_shift_count();
                menu_highlight->set_y(highlight_y0);
                launcher.play_click();
                add_shift(Shift::Type::MENU, Direction::DOWN, card_y0 - current_menu->get_y(), BL::HIGHLIGHT_SHIFT_TIME, menu_objects);
            }
        }
        else {
            // Move highlight left
            add_shift(Shift::Type::HIGHLIGHT, Direction::LEFT, highlight_x_advance, BL::HIGHLIGHT_SHIFT_TIME, {menu_highlight});
            current_menu->dec_column();
            launcher.play_click();
        }
    }
}


void BL::Layout::move_right()
{
    if (selection_mode == SelectionMode::SIDEBAR && current_menu && shift_queue.empty()) {
        selection_mode = SelectionMode::MENU;
        current_menu->reset_row();
        current_entry->set_text_color(config.sidebar_text_color);
        launcher.play_click();
    }

    else if (selection_mode == SelectionMode::MENU) {
        int max_columns = current_menu->num_entries() - current_menu->get_row() * current_menu->get_max_columns();
        if (max_columns > current_menu->get_max_columns())
            max_columns = current_menu->get_max_columns();
        if(current_menu->get_column() < max_columns - 1) {
            
            // Shift highlight right
            add_shift(Shift::Type::HIGHLIGHT, Direction::RIGHT, highlight_x_advance, HIGHLIGHT_SHIFT_TIME, {menu_highlight});
            current_menu->inc_column();
            launcher.play_click();
        }
    }
}

void BL::Layout::select()
{
    if (selection_mode == SelectionMode::SIDEBAR) {
        if (const std::string *command = current_entry->get_command(); command) {
            launcher.execute_command(*command);
            launcher.play_select();
        }
    }
    else if (selection_mode == SelectionMode::MENU) {
        MenuEntry &entry = current_menu->get_current_entry();
        BL::logger::debug("User selected entry '{}'", entry.get_title());
        add_press(entry);
        launcher.play_select();
    }
}

void BL::Layout::add_shift(Shift::Type type, Direction direction, float target, float time, const std::vector<BL::Object*> &objects, Shift::Method method)
{
    static const std::array<Direction, 4> opposites {{
        Direction::DOWN,
        Direction::UP,
        Direction::RIGHT,
        Direction::LEFT
    }};
    Direction opposite = opposites[static_cast<int>(direction)];

    // Interrupt if opposite direction shift is in progress
    for (Shift &shift : shift_queue) {
        if (shift.type == type) {
            if (shift.direction == opposite) {
                if (method == BL::Layout::Shift::Method::REL)
                    shift.target = target - (shift.target - shift.total);
                else {
                    shift.target = target;
                    shift.velocity = shift.target / time;
                }
                shift.total = 0.f;
                shift.direction = direction;
                shift.ticks = SDL_GetTicks();
                shift.method = method;
                return;
            }
            else if (shift.direction == direction) {
                shift.target = method == BL::Layout::Shift::Method::ABS ? target : (shift.target - shift.total) + target;
                shift.method = method;
                shift.ticks = SDL_GetTicks();
                shift.total = 0.f;
                shift.velocity = (shift.velocity + shift.target / time) / 2.f;
                return;
            }
        }
    }

    // Add new shift to queue
    shift_queue.emplace_back(
            type,
            method,
            direction,
            target / time,
            target,
            objects
    );
}


void BL::Layout::update_shift()
{
    Uint64 ticks = SDL_GetTicks();
    for (auto shift = shift_queue.begin(); shift != shift_queue.end();) {
        
        // Calculate position change based on velocity and time elapsed
        float current =  (static_cast<float>(ticks - shift->ticks)) * shift->velocity;
        if (shift->total + current > shift->target)
            current = shift->target - shift->total;
        shift->total += current;
        if (shift->direction == Direction::UP || shift->direction == Direction::LEFT)
            current *= -1.f;

        if (shift->direction == Direction::UP || shift->direction == Direction::DOWN) {
            for (BL::Object *object : shift->objects)
                object->inc_y(current);
        }
        else {
            for (BL::Object *object : shift->objects)
                object->inc_x(current);         
        }
        shift->ticks = ticks;
        shift = shift->target == shift->total ? shift_queue.erase(shift) : shift + 1;
    }
}

void BL::Layout::update_press()
{
    for (auto press = press_queue.begin(); press != press_queue.end();) {
        Uint64 current_ticks = SDL_GetTicks();
        float change = (static_cast<float>(current_ticks - press->ticks) * press->velocity);
        if (press->direction == Direction::RIGHT) {
            press->current += change;
            if (press->current >= press->total) {
                press->current = press->total;
                press->direction = Direction::LEFT;
            }
        }
        else if (press->direction == Direction::LEFT) {
            press->current -= change;
            if (press->current <= 0.f) {
                press->entry->set_x(press->original_rect.x);
                press->entry->set_y(press->original_rect.y);
                press->entry->set_w(press->original_rect.w);
                press->entry->set_h(press->original_rect.h);
                launcher.execute_command(press->entry->get_command());
                press = press_queue.erase(press);
                continue;
            }
        }
        float w = press->original_rect.w - 2.f * press->current;
        float h = w / press->aspect_ratio;
        press->entry->set_x(press->original_rect.x + press->current);
        press->entry->set_y(press->original_rect.y + (press->current / press->aspect_ratio));
        press->entry->set_w(w);
        press->entry->set_h(h);
        press->ticks = current_ticks;
        ++press;
    }
}

void BL::Layout::update()
{
    if (!shift_queue.empty())
        update_shift();
    if (!press_queue.empty())
        update_press();
    if (screensaver)
        screensaver->update();
}

void BL::Layout::draw()
{
    renderer->clear();

    // Draw background
    if (background_texture)
        renderer->draw(*background_texture);

    // Draw sidebar highlight
    if (selection_mode == SelectionMode::SIDEBAR) {
        renderer->set_clip_rect(sidebar_highlight_clip);
        sidebar_highlight->draw();
    }
    
    // Draw sidebar texts
    renderer->set_clip_rect(sidebar_text_clip);
    for (BL::SidebarEntry &entry : sidebar_entries)
        entry.draw();

    // Draw menu entries
    renderer->set_clip_rect(menu_clip);
    for (BL::Menu &menu : menus) {
        if (menu.is_visible(y_min, y_max))
            menu.draw();
    }
    renderer->disable_clip();

    // Draw menu highlight
    if (selection_mode == SelectionMode::MENU)
        menu_highlight->draw();

    // Draw screensaver
    if (screensaver && screensaver->is_active())
        screensaver->draw();

    // Output to screen
    renderer->present();
}

BL::Layout::Layout(const std::string &file, int w, int h, Launcher &launcher):
    screen_width(w),
    screen_height(h),
    launcher(launcher),
    rasterizer(new BL::SVGRasterizer())
{
    parse(file);
}
BL::Layout::~Layout()
{
    delete error_texture;
    delete background_texture;
    delete sidebar_highlight;
    delete menu_highlight;
    delete screensaver;
}
