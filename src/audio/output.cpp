#include <SDL3/SDL.h>
#include <algorithm>
#include <touch/audio/types.hpp>
namespace touch {
namespace {
class SdlOutput : public AudioOutput {
    SDL_AudioStream* stream = nullptr;

  public:
    SdlOutput() {
        SDL_AudioSpec spec{SDL_AUDIO_F32, 2, 48000};
        stream =
            SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        if (!stream)
            throw std::runtime_error(SDL_GetError());
        SDL_ResumeAudioStreamDevice(stream);
    }
    ~SdlOutput() override {
        SDL_DestroyAudioStream(stream);
    }
    void write(Value a, float gain) override {
        if (a->components > 2)
            throw std::runtime_error("Add Stereo Mix before Audio Out");
        if (SDL_GetAudioStreamQueued(stream) >= 48000 * 2 * 4 / 4)
            return;
        std::vector<float> stereo(960);
        for (int i = 0; i < 480; ++i)
            for (int ch = 0; ch < 2; ++ch)
                stereo[i * 2 + ch] = std::clamp(
                    a->values[i * a->components + (a->components == 1 ? 0 : ch)] * gain, -1.f, 1.f);
        if (!SDL_PutAudioStreamData(stream, stereo.data(), stereo.size() * sizeof(float)))
            throw std::runtime_error(SDL_GetError());
    }
};
} // namespace
std::unique_ptr<AudioOutput> SdlAudioOutputs::open() {
    return std::make_unique<SdlOutput>();
}
} // namespace touch
