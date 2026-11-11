#include "decoder.hpp"
namespace touch::media {
void Decoder::receive(Codec& codec, int index) {
    while (avcodec_receive_frame(codec.get(), frame.get()) >= 0) {
        double pts =
            frame->best_effort_timestamp == AV_NOPTS_VALUE
                ? 0
                : frame->best_effort_timestamp * av_q2d(format->streams[index]->time_base) - origin;
        if (frame->width < 0 || frame->height < 0 ||
            uint64_t(frame->width) * frame->height > 16777216)
            throw std::runtime_error("Decoded image exceeds size limit");
        if (index == vi) {
            auto b = std::const_pointer_cast<Data>(data(Signal::Video, w, h, 4));
            float scale = std::min(float(w) / frame->width, float(h) / frame->height);
            int rw = std::max(1, int(std::round(frame->width * scale))),
                rh = std::max(1, int(std::round(frame->height * scale)));
            std::vector<uint8_t> pixels(size_t(rw) * rh * 4);
            scaler = sws_getCachedContext(scaler, frame->width, frame->height,
                                          AVPixelFormat(frame->format), rw, rh, AV_PIX_FMT_RGBA,
                                          SWS_BILINEAR, nullptr, nullptr, nullptr);
            if (!scaler)
                throw std::runtime_error("Cannot create video scaler");
            uint8_t* dst[] = {pixels.data()};
            int stride[] = {rw * 4};
            sws_scale(scaler, frame->data, frame->linesize, 0, frame->height, dst, stride);
            int left = (w - rw) / 2, top = (h - rh) / 2;
            for (int y = 0; y < rh; ++y)
                for (int x = 0; x < rw; ++x)
                    for (int c = 0; c < 4; ++c)
                        b->values[((y + top) * w + x + left) * 4 + c] =
                            pixels[(y * rw + x) * 4 + c] / 255.f;
            image = b;
            video_time = pts;
        } else {
            if (!resampler) {
                AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
                checked(swr_alloc_set_opts2(&resampler, &stereo, AV_SAMPLE_FMT_FLT, 48000,
                                            &frame->ch_layout, AVSampleFormat(frame->format),
                                            frame->sample_rate, 0, nullptr));
                checked(swr_init(resampler));
            }
            int count = swr_get_out_samples(resampler, frame->nb_samples);
            std::vector<float> samples(size_t(count) * 2);
            uint8_t* dst = reinterpret_cast<uint8_t*>(samples.data());
            int n =
                swr_convert(resampler, &dst, count,
                            const_cast<const uint8_t**>(frame->extended_data), frame->nb_samples);
            checked(n);
            int64_t start = std::llround(pts * 48000);
            for (int i = 0; i < n; ++i)
                audio[start + i] = {samples[i * 2], samples[i * 2 + 1]};
        }
        av_frame_unref(frame.get());
    }
}
} // namespace touch::media
