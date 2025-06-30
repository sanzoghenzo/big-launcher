#pragma once

#include <string>
#include <mutex>
#include <SDL3/SDL.h>

namespace BL {
    class SoundFile {  
    private:
        Uint8 *buffer = nullptr;
        Uint32 len = 0;
        Uint32 pos = 0;
    public:
        SoundFile(const std::string &path);
        ~SoundFile();

        const Uint8* get_buffer() const { return buffer; }
        Uint32 get_len() const { return len; }
        Uint32 get_pos() const { return pos; }
        void inc_pos(Uint32 delta_pos) { pos += delta_pos; if (pos >= len) pos = 0; }
        void reset_pos() { pos = 0; }
    };
    class Sound {
        private:
            enum State {
                IDLE,
                CLICK_PLAYING,
                SELECT_PLAYING
            };
            SoundFile *click = nullptr;
            SoundFile *select = nullptr;
            SoundFile *current_file = nullptr;
            SDL_AudioStream *stream = nullptr;
            State state = State::IDLE;
            std::mutex mutex;

            void set_current_file(SoundFile &file);

        public:

            Sound();
            ~Sound();
            static void callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
            void put_data(int amount);
            bool connect();
            void disconnect();
            void play_click();
            void play_select();
    };
}

