#pragma once
#include <touch/operators/types.hpp>
namespace touch {
class MediaSource {
  public:
    virtual ~MediaSource() = default;
    virtual std::pair<Value, Value> read(double) = 0;
};
class MediaFactory {
  public:
    virtual ~MediaFactory() = default;
    virtual std::unique_ptr<MediaSource> open(const Node&, const Context&) = 0;
};
class FFmpegMedia : public MediaFactory {
  public:
    std::unique_ptr<MediaSource> open(const Node&, const Context&) override;
};

} // namespace touch
