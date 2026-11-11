#include <SDL3/SDL.h>
#include <touch/media/types.hpp>
namespace touch::media {
class Microphone : public MediaSource {
    SDL_AudioStream* stream = nullptr;

  public:
    Microphone() {
        SDL_AudioSpec spec{SDL_AUDIO_F32, 2, 48000};
        stream =
            SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, nullptr, nullptr);
        if (!stream)
            throw std::runtime_error(SDL_GetError());
        SDL_ResumeAudioStreamDevice(stream);
    }
    ~Microphone() override {
        SDL_DestroyAudioStream(stream);
    }
    std::pair<Value, Value> read(double) override {
        auto b = std::const_pointer_cast<Data>(data(Signal::Audio, 480, 1, 2));
        if (SDL_GetAudioStreamData(stream, b->values.data(),
                                   int(b->values.size() * sizeof(float))) < 0)
            throw std::runtime_error(SDL_GetError());
        return {nullptr, b};
    }
};
std::unique_ptr<MediaSource> microphone() {
    return std::make_unique<Microphone>();
}
} // namespace touch::media
