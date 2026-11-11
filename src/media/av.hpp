#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <touch/media/types.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
namespace touch::media {
inline std::string av_error(int code) {
    char text[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(code, text, sizeof(text));
    return text;
}
inline void checked(int code) {
    if (code < 0)
        throw std::runtime_error(av_error(code));
}
struct FormatDeleter {
    void operator()(AVFormatContext* p) const {
        avformat_close_input(&p);
    }
};
struct CodecDeleter {
    void operator()(AVCodecContext* p) const {
        avcodec_free_context(&p);
    }
};
struct FrameDeleter {
    void operator()(AVFrame* p) const {
        av_frame_free(&p);
    }
};
struct PacketDeleter {
    void operator()(AVPacket* p) const {
        av_packet_free(&p);
    }
};
using Codec = std::unique_ptr<AVCodecContext, CodecDeleter>;
} // namespace touch::media
