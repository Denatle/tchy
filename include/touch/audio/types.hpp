#pragma once
#include <chrono>
#include <thread>
#include <touch/graph/types.hpp>
namespace touch {
class AudioOutput {
  public:
    virtual ~AudioOutput() = default;
    virtual void write(Value, float gain) = 0;
};
class AudioOutputs {
  public:
    virtual ~AudioOutputs() = default;
    virtual std::unique_ptr<AudioOutput> open() = 0;
};
class SdlAudioOutputs : public AudioOutputs {
  public:
    std::unique_ptr<AudioOutput> open() override;
};
class Clock {
  public:
    using Time = std::chrono::steady_clock::time_point;
    virtual ~Clock() = default;
    virtual Time now() const = 0;
    virtual void sleep_until(Time) = 0;
};
class SteadyClock : public Clock {
  public:
    Time now() const override {
        return std::chrono::steady_clock::now();
    }
    void sleep_until(Time t) override {
        std::this_thread::sleep_until(t);
    }
};
} // namespace touch
