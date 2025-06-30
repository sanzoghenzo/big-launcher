#include <filesystem>
#include <string>
#include <string_view>
#include <cstring>
#include <SDL3/SDL.h>
#include "logger.hpp"
#include <lconfig.h>
#include "sound.hpp"
#include "main.hpp"
#include "util.hpp"
#include "gamepad.hpp"
#include "config.hpp"

extern const char *executable_dir;

// Calculates the length of a utf-8 encoded C-style string
int BL::utf8_length(std::string_view string)
{
    int length = 0;
    auto it = string.cbegin();
    while (it < string.cend()) {
        if ((*it & 0x80) == 0) // If byte is 0xxxxxxx, then it's a 1 byte (ASCII) char
            it++;
        else if ((*it & 0xE0) == 0xC0) // If byte is 110xxxxx, then it's a 2 byte char
            it +=2;
        else if ((*it & 0xF0) == 0xE0) // If byte is 1110xxxx, then it's a 3 byte char
            it +=3;
        else if ((*it & 0xF8) == 0xF0) // If byte is 11110xxx, then it's a 4 byte char
            it+=4;

        length++;
    }
    return length;
}

// A function to extract the Unicode code point from the first character in a UTF-8 encoded C-style string
Uint16 BL::get_unicode_code_point(const char *p, int &bytes)
{
    Uint16 result;

    // 1 byte ASCII char
    if ((*p & 0x80) == 0) {
        result = (Uint16) *p;
        bytes = 1;
    }

    // If byte is 110xxxxx, then it's a 2 byte char
    else if ((*p & 0xE0) == 0xC0) {
        Uint8 byte1 = *p & 0x1F;
        Uint8 byte2 = *(p + 1) & 0x3F;
        result = (Uint16) ((byte1 << 6) + byte2);
        bytes = 2;
    }

    // If byte is 1110xxxx, then it's a 3 byte char
    else if ((*p & 0xF0) == 0xE0) {
        Uint8 byte1 = *p & 0x0F;
        Uint8 byte2 = *(p + 1) & 0x3F;
        Uint8 byte3 = *(p + 2) & 0x3F;
        result = (Uint16) ((byte1 << 12) + (byte2 << 6) + byte3);
        bytes = 3;
    }
    else {
        result = 0;
        bytes = 1;
    }
    return result;
}

// A function to truncate a utf-8 encoded string to max number of pixels
std::string BL::utf8_truncate(const std::string &string, int width, int max_width)
{
    int string_length = utf8_length(string);
    int avg_width = width / string_length;
    int num_chars = max_width / avg_width;
    int spaces = (string_length - num_chars) + 3; // Number of spaces to go back
    auto ptr = string.cend();
    int chars = 0;

    // Go back required number of spaces
    do {
        ptr--;
        if (!(*ptr & 0x80)) // ASCII characters have 0 as most significant bit
            chars++;
        else { // Non-ASCII character detected
            do {
                ptr--;
            } while (ptr > string.begin() && (*ptr & 0xC0) == 0x80); // Non-ASCII most significant byte begins with 0b11
            chars++;
        }
    } while (chars < spaces);

    // Add "..." to end of string to inform user of truncation
    return std::string(string, 0, (ptr - string.cbegin())) + "...";
}

// A function to convert a hex-formatted string into a color struct
bool BL::hex_to_color(std::string_view string, SDL_Color &color)
{
    auto it = string.cbegin();
    if (*it != '#' || string.size() != 7)
        return false;
    it++;

    Uint32 hex = (Uint32) strtoul(&*it, nullptr, 16);
    if (!hex && strcmp(&*it, "000000"))
        return false;

    color = {
        (Uint8) (hex >> 16),
        (Uint8) ((hex & 0x0000ff00) >> 8),
        (Uint8) (hex & 0x000000ff),
        0xFF
    };
    return true;
}

template <BL::FileType file_type>
std::string BL::find_file(const char *filename)
{
    static std::vector<std::filesystem::path> paths;
    if constexpr(file_type == BL::FileType::CONFIG) {
        if (!paths.size()) {
#ifdef __unix__
            paths.resize(4);
            paths[0] = CURRENT_DIRECTORY;
            paths[1] = executable_dir;
            paths[2] = BL::join_paths(getenv("HOME"), ".config", EXECUTABLE_TITLE);
            paths[3] = SYSTEM_SHARE_DIR;
#endif
#ifdef _WIN32
            paths.resize(2);
            paths[0] = ".\\";
            paths[1] = executable_dir;
#endif
        }
    }

    if constexpr(file_type == BL::FileType::FONT) {
        if (!paths.size()) {
#ifdef __unix__
            paths.resize(2);
            paths[0] = BL::join_paths(executable_dir, "assets", "fonts");
            paths[1] = SYSTEM_FONTS_DIR;

#endif
#ifdef _WIN32
            paths.resize(2);
            paths[0] = ".\\assets\\fonts";
            paths[1] = BL::join_paths(executable_dir, "assets", "fonts");
#endif
        }
    }

    if constexpr(file_type == BL::FileType::AUDIO) {
        if (!paths.size()) {
#ifdef __unix__
            paths.resize(2);
            paths[0] = BL::join_paths(executable_dir, "assets", "sounds");
            paths[1] = SYSTEM_SOUNDS_DIR;

#endif
#ifdef _WIN32
            paths.resize(2);
            paths[0] = ".\\assets\\sounds";
            paths[1] = BL::join_paths(executable_dir, "assets", "sounds");
#endif
        }
    }
    for (const std::filesystem::path &path : paths) {
        auto file = join_paths(path, filename);
        if (std::filesystem::exists(file))
            return file.string();
    }
    return std::string();
}
template std::string BL::find_file<BL::FileType::CONFIG>(const char *filename);
template std::string BL::find_file<BL::FileType::FONT>(const char *filename);
template std::string BL::find_file<BL::FileType::AUDIO>(const char *filename);
