#pragma once
#include "av.hpp"
namespace touch::media {
class Decoder : public MediaSource {
    std::unique_ptr<AVFormatContext, FormatDeleter> format;
    Codec video_codec, audio_codec;
    std::unique_ptr<AVFrame, FrameDeleter> frame{av_frame_alloc()};
    std::unique_ptr<AVPacket, PacketDeleter> packet{av_packet_alloc()};
    SwsContext* scaler = nullptr;
    SwrContext* resampler = nullptr;
    int vi = -1, ai = -1, w, h;
    bool ended = false, still = false;
    double video_time = -1, origin = 0;
    Value image;
    std::map<int64_t, std::array<float, 2>> audio;
    std::atomic<int64_t> deadline{0};
    static int interrupt(void*);
    void renew_deadline();
    Codec open_codec(int);
    void receive(Codec&, int);

  public:
    Decoder(const Node&, const Context&);
    ~Decoder() override;
    std::pair<Value, Value> read(double) override;
};
} // namespace touch::media
