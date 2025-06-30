#include <string>
#include <cmath>
#include <bit>

#include <SDL3/SDL.h>
#include "logger.hpp"

#include <lconfig.h>
#include "sound.hpp"
#include "config.hpp"
#include "util.hpp"

extern BL::Config config;
namespace BL {
    constexpr double RANGE_DB = 40.0;
}

BL::SoundFile::SoundFile(const std::string &path)
{
    SDL_AudioSpec spec;
    SDL_LoadWAV(path.c_str(), &spec, &buffer, &len);
    if (spec.format != SDL_AUDIO_S16LE || spec.channels != 1 || spec.freq != 48000)
        BL::logger::error_throw("Audio file {} has invalid format", path);

    // set volume
    if (config.sound_volume <= 0 || config.sound_volume >= BL::MAX_VOLUME)
        return;
    float scale_factor = static_cast<float>(std::pow(10.0, (static_cast<double>(config.sound_volume - BL::MAX_VOLUME) * (BL::RANGE_DB / static_cast<double>(BL::MAX_VOLUME))) / 20.0));
    Sint16 *data = reinterpret_cast<Sint16*>(buffer);
    size_t nb_samples = len / sizeof(Sint16);
    for (size_t i = 0; i < nb_samples; i++) {
        if constexpr (std::endian::native == std::endian::big)
            data[i] = std::byteswap<Sint16>(static_cast<Sint16>(std::round(static_cast<float>(std::byteswap<Sint16>(data[i])) * scale_factor)));
        else
            data[i] = static_cast<Sint16>(std::round(static_cast<float>(data[i]) * scale_factor));
    }
}

BL::SoundFile::~SoundFile()
{
    SDL_free(buffer);
}

BL::Sound::Sound()
{
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        BL::logger::error_throw("Failed to initialize audio");
    }
    BL::logger::debug("Successfully initialized audio");

    std::string click_path = BL::find_file<BL::FileType::AUDIO>(CLICK_FILENAME);
    if (click_path.empty())
        BL::logger::error_throw("Could not locate audio file '{}'", CLICK_FILENAME);
    std::string select_path = BL::find_file<BL::FileType::AUDIO>(SELECT_FILENAME);
    if (select_path.empty())
        BL::logger::error_throw("Could not locate audio file '{}'", SELECT_FILENAME);
    click = new BL::SoundFile(click_path);
    select = new BL::SoundFile(select_path);

    connect();
}

BL::Sound::~Sound()
{
    disconnect();
    delete click;
    delete select;
}

bool BL::Sound::connect()
{
    BL::logger::debug("Opening audio device...");
    SDL_AudioSpec spec = { SDL_AUDIO_S16LE, 1, 48000 };
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &BL::Sound::callback, this);
    if (!stream || !SDL_ResumeAudioStreamDevice(stream)) {
        BL::logger::error("Failed to open audio device (SDL Error: {})", SDL_GetError());
        return false;
    }
    SDL_ResumeAudioStreamDevice(stream);
    BL::logger::debug("Successfully opened audio device");
    return true;
}

void BL::Sound::disconnect()
{
    if (stream){
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }
}

void BL::Sound::callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    reinterpret_cast<BL::Sound*>(userdata)->put_data(additional_amount);
}

void BL::Sound::put_data(int amount)
{
    std::unique_lock lock(mutex);
    if (!current_file)
        return;
    int bytes_read = std::min(amount, static_cast<int>(current_file->get_len() - current_file->get_pos()));
    SDL_PutAudioStreamData(stream, reinterpret_cast<const void*>(current_file->get_buffer() + current_file->get_pos()), bytes_read);
    current_file->inc_pos(bytes_read);
    if (!current_file->get_pos())
        current_file = nullptr;
}

void BL::Sound::set_current_file(BL::SoundFile &file)
{
    if (!stream)
        return;
    std::unique_lock lock(mutex);
    current_file = &file;
    current_file->reset_pos();
}

void BL::Sound::play_click()
{
    set_current_file(*click);
}

void BL::Sound::play_select()
{
    set_current_file(*select);
}
