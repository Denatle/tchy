#include "decoder.hpp"
namespace touch::media {
std::unique_ptr<MediaSource> microphone();
}
namespace touch {
std::unique_ptr<MediaSource> FFmpegMedia::open(const Node& n, const Context& c) {
    if (n.type == "builtin.Microphone")
        return media::microphone();
    return std::make_unique<media::Decoder>(n, c);
}
} // namespace touch
