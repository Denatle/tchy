#include "decoder.hpp"
namespace touch::media {
std::pair<Value, Value> Decoder::read(double seconds) {
    if (!(still && image)) {
        int packets = 0;
        int64_t desired = std::llround(seconds * 48000) + 480;
        while (!ended && packets++ < 1000 &&
               ((vi >= 0 && video_time < seconds) ||
                (ai >= 0 && (audio.empty() || audio.rbegin()->first < desired)))) {
            renew_deadline();
            int rc = av_read_frame(format.get(), packet.get());
            if (rc == AVERROR_EOF) {
                ended = true;
                if (video_codec) {
                    avcodec_send_packet(video_codec.get(), nullptr);
                    receive(video_codec, vi);
                }
                if (audio_codec) {
                    avcodec_send_packet(audio_codec.get(), nullptr);
                    receive(audio_codec, ai);
                }
                break;
            }
            checked(rc);
            int index = packet->stream_index;
            if (index == vi) {
                checked(avcodec_send_packet(video_codec.get(), packet.get()));
                receive(video_codec, vi);
            }
            if (index == ai) {
                checked(avcodec_send_packet(audio_codec.get(), packet.get()));
                receive(audio_codec, ai);
            }
            av_packet_unref(packet.get());
            if (still && image)
                break;
        }
    }
    auto block = std::const_pointer_cast<Data>(data(Signal::Audio, 480, 1, 2));
    block->names = {"left", "right"};
    int64_t start = std::llround(seconds * 48000);
    for (int i = 0; i < 480; ++i) {
        auto it = audio.find(start + i);
        if (it != audio.end()) {
            block->values[i * 2] = it->second[0];
            block->values[i * 2 + 1] = it->second[1];
        }
    }
    audio.erase(audio.begin(), audio.lower_bound(start + 480));
    return {image, ai >= 0 ? block : nullptr};
}
} // namespace touch::media
