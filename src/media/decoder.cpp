#include "decoder.hpp"
namespace touch::media {
Decoder::Decoder(const Node& n, const Context& c) : w(c.width), h(c.height) {
    avdevice_register_all();
    if (!frame || !packet)
        throw std::bad_alloc();
    const AVInputFormat* device = nullptr;
    std::string path = n.file;
    AVDictionary* opts = nullptr;
    still = n.type == "builtin.Image";
    if (n.type == "builtin.Camera") {
#ifdef __APPLE__
        device = av_find_input_format("avfoundation");
        path = n.file.empty() ? "default:none" : n.file;
#elif defined(_WIN32)
        device = av_find_input_format("dshow");
        if (path.empty())
            throw std::runtime_error("Set camera source to video=<device name>");
#else
        device = av_find_input_format("video4linux2");
        if (path.empty())
            path = "/dev/video0";
#endif
        av_dict_set(&opts, "framerate", "30", 0);
    }
    AVFormatContext* input = avformat_alloc_context();
    if (!input)
        throw std::bad_alloc();
    input->interrupt_callback = {interrupt, this};
    renew_deadline();
    int result = avformat_open_input(&input, path.c_str(), device, &opts);
    av_dict_free(&opts);
    if (result < 0) {
        if (input)
            avformat_close_input(&input);
        checked(result);
    }
    format.reset(input);
    checked(avformat_find_stream_info(input, nullptr));
    origin = input->start_time == AV_NOPTS_VALUE ? 0 : double(input->start_time) / AV_TIME_BASE;
    vi = av_find_best_stream(input, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    ai = av_find_best_stream(input, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (n.type == "builtin.AudioFile")
        vi = -1;
    if (still)
        ai = -1;
    if (vi >= 0)
        video_codec = open_codec(vi);
    if (ai >= 0)
        audio_codec = open_codec(ai);
    if (vi < 0 && ai < 0)
        throw std::runtime_error("No usable media tracks");
}
Decoder::~Decoder() {
    sws_freeContext(scaler);
    swr_free(&resampler);
}
Codec Decoder::open_codec(int index) {
    auto stream = format->streams[index];
    auto decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if ((stream->codecpar->codec_id == AV_CODEC_ID_VP8 ||
         stream->codecpar->codec_id == AV_CODEC_ID_VP9) &&
        av_dict_get(stream->metadata, "alpha_mode", nullptr, 0)) {
        auto alpha = avcodec_find_decoder_by_name(
            stream->codecpar->codec_id == AV_CODEC_ID_VP9 ? "libvpx-vp9" : "libvpx");
        if (!alpha)
            throw std::runtime_error("FFmpeg requires libvpx to preserve this movie's alpha");
        decoder = alpha;
    }
    if (!decoder)
        throw std::runtime_error("Decoder unavailable");
    Codec context(avcodec_alloc_context3(decoder));
    checked(avcodec_parameters_to_context(context.get(), stream->codecpar));
    checked(avcodec_open2(context.get(), decoder, nullptr));
    return context;
}
void Decoder::renew_deadline() {
    deadline.store(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            (std::chrono::steady_clock::now() + std::chrono::seconds(2)).time_since_epoch())
            .count());
}
int Decoder::interrupt(void* ctx) {
    auto self = static_cast<Decoder*>(ctx);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
               .count() > self->deadline.load();
}
} // namespace touch::media
