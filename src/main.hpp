#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <cmath>


#define DISPLAY_ASPECT_RATIO 1.77777778
#define DISPLAY_ASPECT_RATIO_TOLERANCE 0.01f
#define APPLICATION_WAIT_PERIOD 100
#define APPLICATION_TIMEOUT 10000

namespace BL {
    class Renderer;
    class Layout;
    class SVGRasterizer;
    class Gamepad;
    class Sound;
    class Config;
    class Launcher {
    private:
        struct Ticks {
            Uint32 main;
            Uint32 application_launch;
            Uint32 last_input;
        };
        struct State {
            bool application_launching = false;
            bool application_running = false;
        };
        Layout *layout;
        Renderer *renderer = nullptr;
        Gamepad *gamepad = nullptr;
        Sound *sound = nullptr;
        Ticks ticks{};
        State state;
        SDL_Window *window = nullptr;
        const SDL_DisplayMode *dm = nullptr;
        int render_w = 0;
        int render_h = 0;
        bool letterbox = false;
        bool quit = false;

        void init_logging();
        void locate_files();
        void init_display();
        void create_window();
        void debug_display();
        void pre_launch();
        void post_launch();

    public:
        Launcher(const std::string &layout_path);
        ~Launcher();

        int run();
        void execute_command(const std::string &command);
        void play_click();
        void play_select();
        Uint64 current_time() const { return ticks.main; }
        Uint64 time_since_last_input() const { return ticks.main - ticks.last_input; }
#ifdef _WIN32
        static bool process_msg(void* userdata, MSG* msg);
#endif
    };
}
