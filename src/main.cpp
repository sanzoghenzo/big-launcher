#include <memory>
#include <string>
#include <algorithm>
#include <filesystem>
#include <exception>
#include <getopt.h>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#include <SDL3/SDL_main.h>
#endif
#include <fmt/core.h>
#include "logger.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "basic_lazy_file_sink.hpp"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <lconfig.h>
#include "main.hpp"
#include "gamepad.hpp"
#include "layout.hpp"
#include "hotkey.hpp"
#include "renderer.hpp"
#include "renderer_sdl.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "config.hpp"
#include "platform/platform.hpp"

namespace BL {
    enum class Output {
        Log,
        Console
    };
    std::pair<std::string, std::string> parse_command_line(int argc, char* argv[]);
    std::shared_ptr<spdlog::logger> init_logging();
}

BL::Config config;
std::string log_path;
const char *executable_dir = SDL_GetBasePath();

template<BL::Output out, typename... Args>
inline void log_or_print(fmt::format_string<Args...> fmt, Args... args)
{
    if constexpr(out == BL::Output::Log)
        spdlog::debug(fmt, std::forward<Args>(args)...);
    else {
        fmt::print(fmt, std::forward<Args>(args)...);
        fmt::print("\n");
    }
}
template<BL::Output out>
void print_version()
{
    int sdl_version = SDL_GetVersion();
    int img_version = IMG_Version();
    int ttf_version = TTF_Version();
    log_or_print<out>(PROJECT_NAME " version " PROJECT_VERSION ", using:");
    log_or_print<out>("    SDL        {}.{}.{}", SDL_VERSIONNUM_MAJOR(sdl_version), SDL_VERSIONNUM_MINOR(sdl_version), SDL_VERSIONNUM_MICRO(sdl_version));
    log_or_print<out>("    SDL_image  {}.{}.{}", SDL_VERSIONNUM_MAJOR(img_version), SDL_VERSIONNUM_MINOR(img_version), SDL_VERSIONNUM_MICRO(img_version));
    log_or_print<out>("    SDL_ttf    {}.{}.{}", SDL_VERSIONNUM_MAJOR(ttf_version), SDL_VERSIONNUM_MINOR(ttf_version), SDL_VERSIONNUM_MICRO(ttf_version));
    log_or_print<out>("");
    log_or_print<out>("Build date: " __DATE__);
#ifdef __GNUC__
    log_or_print<out>("Compiler:   GCC {}.{}", __GNUC__, __GNUC_MINOR__);
#endif

#ifdef _MSC_VER
    log_or_print<out>("Compiler:   Microsoft C/C++ {:.2f}", (float) _MSC_VER / 100.0f);
#endif
}

BL::Launcher::Launcher(const std::string &layout_path)
{
    init_display();
    layout = new BL::Layout(layout_path, render_w, render_h, *this);
    layout->load_surfaces();

    create_window();

    BL::logger::debug("Creating renderer...");
    renderer = new BL::RendererSDL(*window);
#ifdef DEBUG
    if (config.render_w && config.render_h)
        renderer->set_render_scale(static_cast<float>(dm->w)/ static_cast<float>(render_w), static_cast<float>(dm->h)/static_cast<float>(render_h));
#endif
    if (letterbox)
        renderer->set_logical_representation(render_w, render_h);
    layout->load_textures(*renderer);
    if (config.sound_enabled) {
        try {
            sound = new BL::Sound();
        }
        catch(...) {
            delete sound;
            sound = nullptr;
        }
    }
    if (config.gamepad_enabled) {
        try {
            gamepad = new BL::Gamepad(1000 / dm->refresh_rate, config.gamepad_mappings_file, *this);
        }
        catch(...) {
            delete gamepad;
            gamepad = nullptr;
        }
    }

#ifdef _WIN32
    if (has_exit_hotkey()) {
        register_exit_hotkey();
        SDL_SetWindowsMessageHook(&BL::Launcher::process_msg, nullptr);
    }
#endif
}

BL::Launcher::~Launcher()
{
    delete layout;
    delete renderer;
    delete gamepad;
    delete sound;
    if (window)
        SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
}

#ifdef _WIN32
bool BL::Launcher::process_msg(void* userdata, MSG *msg)
{
    if (msg->message == WM_HOTKEY)
        return !close_foreground_window();
    return true;
}

#endif

void BL::Launcher::play_click()
{
    if (sound)
        sound->play_click();
}

void BL::Launcher::play_select()
{
    if (sound)
        sound->play_select();
}

#ifdef __unix__
static void print_help()
{
    fmt::print("Usage: " EXECUTABLE_TITLE " [OPTIONS]\n");
    fmt::print("    -c p,  --config=p     Load config file from path p.\n");
    fmt::print("    -l p,  --layout=p     Load layout file from path p.\n");
    fmt::print("    -d,    --debug        Enable debug messages.\n");
#ifdef DEBUG
    fmt::print("    -r WxH, -resolution   Render layout at WxH resolution.\n");
#endif
    fmt::print("    -h,    --help         Show this help message.\n");
    fmt::print("    -v,    --version      Print version information.\n");
}
#endif

#ifdef DEBUG
static inline void parse_render_resolution(const char *string, int &w, int &h)
{
    std::string s = string;
    size_t x = s.find_first_of('x');
    if (x == std::string::npos) {
        BL::logger::error("Invalid resolution argument '{}'", string);
        return;
    }
    w = std::atoi(s.substr(0, x).c_str());
    h = std::atoi(s.substr(x + 1, s.size() - x).c_str());
    if (!w || !h)
        BL::logger::error("Invalid resolution argument '{}'", string);
}
#endif

std::pair<std::string, std::string> BL::parse_command_line(int argc, char* argv[])
{
    int c;
    const char *short_opts = "+c:l:d"
#ifdef DEBUG
    "r:"
#endif
#ifdef __unix__
    "hv"
#endif
    ;
    std::string config_path;
    std::string layout_path;
    static struct option long_opts[] = {
        { "config",       required_argument, nullptr, 'c' },
        { "layout",       required_argument, nullptr, 'l' },
        { "debug",        no_argument,       nullptr, 'd' },
#ifdef DEBUG
        { "resolution",   required_argument,  nullptr, 'r' },
#endif
#ifdef __unix__

        { "help",         no_argument,       nullptr, 'h' },
        { "version",      no_argument,       nullptr, 'v' },
#endif              
        { 0, 0, 0, 0 }
    };
  
    while ((c = getopt_long(argc, argv, short_opts, long_opts, nullptr)) !=-1) {
        switch (c) {
            case 'c':
                config_path = optarg;
                break;  

            case 'l':
                layout_path = optarg;
                break;

            case 'd':
                config.debug = true;
                break;
#ifdef DEBUG
            case 'r':
                {
                    int w = 0;
                    int h = 0;
                    parse_render_resolution(optarg, w, h);
                    if (!w || !h)
                        throw std::runtime_error(fmt::format("Invalid resolution '{}'\n", optarg));
                    config.render_w = w;
                    config.render_h = h;
                }

                break;
#endif
#ifdef __unix__
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                break;

            case 'v':
                print_version<BL::Output::Console>();
                exit(EXIT_SUCCESS);
                break;
#endif              
        }
    }
    return {std::move(config_path), std::move(layout_path)};
}

std::shared_ptr<spdlog::logger> BL::init_logging()
{
    // Initialize logging
#ifdef __unix__
    char *home_dir = getenv("HOME");
    log_path = BL::join_paths(home_dir, ".local", "share", EXECUTABLE_TITLE, LOG_FILENAME).string();
#endif
#ifdef _WIN32
    log_path = BL::join_paths(executable_dir, LOG_FILENAME).string();
#endif
    auto file_sink = std::make_shared<spdlog::sinks::basic_lazy_file_sink_st>(log_path, true);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    std::vector<spdlog::sink_ptr> sinks {file_sink};
#ifdef __unix__
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[%^%l%$] %v");
    sinks.push_back(console_sink);
#endif
    auto logger = std::make_shared<spdlog::logger>(PROJECT_NAME, sinks.begin(), sinks.end());
    logger->set_level((config.debug) ? spdlog::level::debug : spdlog::level::info);
    spdlog::set_default_logger(logger);
    if (config.debug) {
        print_version<BL::Output::Log>();
        BL::logger::debug("");
    }
    return logger;
}

void BL::Launcher::init_display()
{
#ifdef __unix__
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
        BL::logger::critical("Could not initialize SDL (SDL Error: {})", SDL_GetError());

    int display_count;
    SDL_DisplayID *displays = SDL_GetDisplays(&display_count);
    if (!display_count)
        BL::logger::critical("Could not find any valid display");
    SDL_DisplayID display_id = displays[0];
    SDL_free(displays);
     
    if (!(dm = SDL_GetDesktopDisplayMode(display_id)))
        BL::logger::critical("Could not get desktop display mode (SDL Error: {})", SDL_GetError());
#ifdef DEBUG
    if (config.render_w && config.render_h) {
        render_w = config.render_w;
        render_h = config.render_h;
    }
    else {
        render_w = dm->w;
        render_h = dm->h;
    }
#else
    render_w = dm->w;
    render_h = dm->h;
#endif

    // Force 16:9 aspect ratio
    float aspect_ratio = static_cast<float>(render_w) / static_cast<float>(render_h);
    if (aspect_ratio > DISPLAY_ASPECT_RATIO + DISPLAY_ASPECT_RATIO_TOLERANCE) {
        render_w = static_cast<int>(std::round(static_cast<float>(render_h) * DISPLAY_ASPECT_RATIO));
        letterbox = true;
    }
    else if (aspect_ratio < DISPLAY_ASPECT_RATIO - DISPLAY_ASPECT_RATIO_TOLERANCE) {
        render_h = static_cast<int>(std::round(static_cast<float>(render_w) / DISPLAY_ASPECT_RATIO));
        letterbox = true;
    }

    // Initialize SDL_ttf
    if (!TTF_Init())
        BL::logger::critical("Could not initialize SDL_ttf (SDL Error: {})", SDL_GetError());

    BL::logger::debug("Successfully initialized display");
}

void BL::Launcher::create_window()
{
    BL::logger::debug("Creating window...");
    window = SDL_CreateWindow(PROJECT_NAME,
                 dm->w,
                 dm->h,
                 SDL_WINDOW_FULLSCREEN 
             );
    if (!window)
        BL::logger::critical("Could not create window (SDL Error: {})", SDL_GetError());
    BL::logger::debug("Sucessfully created window");
#ifdef _WIN32
    set_hwnd(*window);
#endif
    SDL_HideCursor();

    if (config.debug)
        debug_display();
}

void BL::Launcher::debug_display()
{
    BL::logger::debug("Video Information:");
    BL::logger::debug("  Resolution:   {}x{}", dm->w, dm->h);
    BL::logger::debug("  Refresh Rate: {} Hz", dm->refresh_rate);
    BL::logger::debug("  Driver:       {}", SDL_GetCurrentVideoDriver());
}

// Main program loop
int BL::Launcher::run()
{
    SDL_Event event;
    ticks.main = ticks.last_input = SDL_GetTicks();

    BL::logger::debug("");
    BL::logger::debug("Begin main loop");
    while(!quit) {
        ticks.main = SDL_GetTicks();
        layout->update();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    if (!state.application_launching) {
                        if (event.key.key == SDLK_DOWN)
                            layout->move_down();
                        else if (event.key.key == SDLK_UP)
                            layout->move_up();
                        else if (event.key.key == SDLK_LEFT)
                            layout->move_left();
                        else if (event.key.key == SDLK_RIGHT)
                            layout->move_right();
                        else if (event.key.key == SDLK_RETURN)
                            layout->select();

                        // Check hotkeys
                        else {
                            for (Hotkey &hotkey : config.hotkey_list) {
                                if (hotkey.keycode == event.key.key) {
                                    execute_command(hotkey.command);
                                    break;
                                }
                            }
                        }
                        ticks.last_input = ticks.main;
                        SDL_FlushEvent(SDL_EVENT_KEY_DOWN);
                    }
                    break;

                case SDL_EVENT_JOYSTICK_ADDED:
                    if (!gamepad)
                        break;
                    if (SDL_IsGamepad(event.jdevice.which) == true) {
                        if (config.debug) {
                            BL::logger::debug("Detected gamepad '{}' at device index {}",
                                SDL_GetGamepadNameForID(event.jdevice.which),
                                event.jdevice.which
                            );
                        }
                        gamepad->add(event.jdevice.which);
                    }
                    else if (config.debug)
                        BL::logger::debug("Unrecognized joystick detected at device index {}", event.jdevice.which);
                    break;

                case SDL_EVENT_JOYSTICK_REMOVED:
                    BL::logger::debug("Device {} disconnected", event.jdevice.which);
                    if (gamepad)
                        gamepad->remove(event.jdevice.which);
                    break;

                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    BL::logger::debug("Lost window focus");
                    if (state.application_launching) {
                        pre_launch();
                        state.application_launching = false;
                        state.application_running = true;
                    }
                    break;

                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    BL::logger::debug("Gained window focus");
                    if (state.application_running) {
                        post_launch();
                        state.application_running = false;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (config.mouse_select && event.button.button == SDL_BUTTON_LEFT) {
                        ticks.last_input = ticks.main;
                        layout->select();
                    }
                    break;
            }
        }

        if (gamepad && !state.application_launching) {
            if (gamepad->poll())
                ticks.last_input = ticks.main;
        }

        if (state.application_launching && 
        ticks.main - ticks.application_launch > APPLICATION_TIMEOUT) {
            state.application_launching = false;
        }
        if (state.application_running)
            SDL_Delay(APPLICATION_WAIT_PERIOD);
        else
            layout->draw();
    }
    return EXIT_SUCCESS;
}

void BL::Launcher::execute_command(const std::string &command)
{
    // Special commands
    if (command.front() == ':') {
        if (command.starts_with(":fork")) {
            size_t space = command.find_first_of(' ');
            if (space != std::string::npos) {
                size_t cmd_begin = command.find_first_not_of(' ', space);
                if (cmd_begin != std::string::npos)
                    start_process(command.substr(cmd_begin, command.size() - cmd_begin), false);
            }
        }
        else if (command == ":left")
            layout->move_left();
        else if (command == ":right")
            layout->move_right();
        else if (command == ":up")
            layout->move_up();
        else if (command == ":down")
            layout->move_down();
        else if (command == ":select")
            layout->select();
        else if (command == ":shutdown")
            scmd_shutdown();
        else if (command == ":restart")
            scmd_restart();
        else if (command == ":sleep")
            scmd_sleep();
        else if (command == ":quit")
            quit = true;
    }

    // Application launching
    else {
        BL::logger::debug("Executing command '{}'", command);
        state.application_launching = start_process(command, true);
        if (state.application_launching) {
            BL::logger::debug("Successfully executed command");
            ticks.application_launch = ticks.main;
        }
        else
            BL::logger::error("Failed to execute command");
    }
}

void BL::Launcher::pre_launch()
{
    if (sound)
        sound->disconnect();
    if (gamepad)
        gamepad->disconnect();
}

void BL::Launcher::post_launch()
{
    if (sound)
        sound->connect();
    if (gamepad)
        gamepad->connect();
}

int main(int argc, char *argv[])
{
    try {
        auto [config_path, layout_path] = BL::parse_command_line(argc, argv);
        auto logger = BL::init_logging();

        // Find layout and config files
        if (!layout_path.empty()) {
            if (!std::filesystem::exists(layout_path))
                BL::logger::critical("Layout file '{}' does not exist", layout_path);
        }
        else {
            layout_path = BL::find_file<BL::FileType::CONFIG>(LAYOUT_FILENAME);
            if (layout_path.empty())
                BL::logger::critical("Could not locate layout file");
        }
        if (!config_path.empty()) {
            if (!std::filesystem::exists(config_path))
                BL::logger::critical("Config file '{}' does not exist", config_path);
        }
        else {
            config_path = BL::find_file<BL::FileType::CONFIG>(CONFIG_FILENAME);
            if (config_path.empty())
                BL::logger::critical("Could not locate config file");
        }
        config.parse(config_path);

        BL::Launcher launcher(layout_path);
        return launcher.run();
    }
    catch(std::exception &e) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, 
            PROJECT_NAME, 
            fmt::format("A fatal error occured: {}\n\nCheck the log file for more information.", e.what()).c_str(), 
            nullptr
        );
        return EXIT_FAILURE;
    }
}
