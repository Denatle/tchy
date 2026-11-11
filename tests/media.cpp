#include <iostream>
#include <touch/core.hpp>
int main(int argc, char** argv) {
    try {
        touch::FFmpegMedia factory;
        touch::Node movie;
        movie.type = "builtin.Movie";
        movie.file = argv[1];
        auto source = factory.open(movie, {64, 32});
        bool heard = false;
        for (int tick = 0; tick < 50; ++tick) {
            auto [video, audio] = source->read(tick * .01);
            if (!video || video->width != 64 || video->height != 32)
                throw std::runtime_error("Video frame missing");
            if (!audio)
                throw std::runtime_error("Audio track missing");
            float peak = 0;
            for (float x : audio->values)
                peak = std::max(peak, std::abs(x));
            if (tick < 15 && peak > .001f)
                throw std::runtime_error("Delayed audio played early");
            if (tick > 25 && peak > .05f)
                heard = true;
        }
        if (!heard)
            throw std::runtime_error("Delayed audio never played");
        movie.file = argv[2];
        source = factory.open(movie, {64, 32});
        auto [video, audio] = source->read(0);
        if (!video || audio)
            throw std::runtime_error("Video-only movie contract");
        std::cout << "FFmpeg delayed audio and video-only checks passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
