#include "common.hpp"
namespace touch::ops {
Maybe generators(const std::string& k, const Node& n, const std::vector<Value>& in,
                 const Context& c, const Parameters& p) {
    auto get = [&](size_t i) -> Value {
        if (i >= in.size() || !in[i])
            throw std::runtime_error("Connect input " + std::to_string(i + 1));
        return in[i];
    };
    auto choice = [&](int i) { return n.choice(i); };
    if (k == "Pattern" || k == "ControlImage" || k == "ControlMask" || k == "TableImage" ||
        k == "Waveform" || k == "Spectrum") {
        bool mask = k == "ControlMask";
        auto b = make(mask ? Signal::Mask : Signal::Video, c.width, c.height, mask ? 1 : 4);
        if (!mask)
            for (size_t i = 3; i < b->values.size(); i += 4)
                b->values[i] = 1;
        float gray = 0;
        if (k == "ControlImage" || mask)
            gray = clamp(get(0)->values[0]);
        if (k == "ControlImage" || mask)
            for (int i = 0; i < c.width * c.height; ++i)
                for (int j = 0; j < (mask ? 1 : 3); ++j)
                    b->values[i * b->components + j] = gray;
        if (k == "Pattern") {
            constexpr int palette[7][3] = {{224, 219, 220}, {215, 187, 49}, {49, 171, 183},
                                           {63, 177, 95},   {166, 57, 185}, {210, 57, 67},
                                           {43, 65, 193}};
            for (int y = 0; y < c.height; ++y)
                for (int x = 0; x < c.width; ++x)
                    for (int j = 0; j < 3; ++j)
                        b->values[(y * c.width + x) * 4 + j] =
                            y < c.height * 3 / 4 ? palette[x * 7 / c.width][j] / 255.f
                                                 : float(x) / std::max(1, c.width - 1);
        }
        if (k == "TableImage") {
            auto a = get(0);
            int count = std::min(32, int(a->rows.size()));
            for (int i = 0; i < count; ++i) {
                float v = 0;
                try {
                    v = clamp(std::stof(a->rows[i][1]));
                } catch (...) {
                }
                for (int y = i * c.height / count; y < (i + 1) * c.height / count - 2; ++y)
                    for (int x = 0; x < v * c.width; ++x) {
                        auto off = (y * c.width + x) * 4;
                        b->values[off] = .11f;
                        b->values[off + 1] = .36f;
                        b->values[off + 2] = .58f;
                    }
            }
        }
        if (k == "Waveform" || k == "Spectrum") {
            auto a = get(0);
            std::vector<float> mono(a->width);
            for (int i = 0; i < a->width; ++i)
                for (int ch = 0; ch < a->components; ++ch)
                    mono[i] += a->values[i * a->components + ch] / a->components;
            auto pixel = [&](int x, int y, float r, float g, float blue) {
                if (x < 0 || x >= c.width || y < 0 || y >= c.height)
                    return;
                int i = (y * c.width + x) * 4;
                b->values[i] = r;
                b->values[i + 1] = g;
                b->values[i + 2] = blue;
            };
            if (k == "Waveform") {
                int prior = c.height / 2;
                for (int x = 0; x < c.width; ++x) {
                    int y = int((1 - std::clamp(mono[x * mono.size() / c.width], -1.f, 1.f)) *
                                (c.height - 1) * .5f);
                    for (int j = std::min(y, prior); j <= std::max(y, prior); ++j)
                        pixel(x, j, .12, .72, .52);
                    prior = y;
                }
            } else
                for (int bin = 0; bin < 48; ++bin) {
                    float frequency = 100 * std::pow(160.f, bin / 47.f),
                          coef = 2 * std::cos(tau * frequency / 48000), last = 0, prev = 0;
                    for (size_t i = 0; i < mono.size(); ++i) {
                        float now = mono[i] * (.5f - .5f * std::cos(tau * i / (mono.size() - 1))) +
                                    coef * last - prev;
                        prev = last;
                        last = now;
                    }
                    float level =
                        4 *
                        std::sqrt(std::max(0.f, last * last + prev * prev - coef * last * prev)) /
                        mono.size();
                    int h =
                        int(clamp((20 * std::log10(std::max(level, 1e-6f)) + 60) / 60) * c.height);
                    for (int x = bin * c.width / 48; x < (bin + 1) * c.width / 48 - 2; ++x)
                        for (int y = c.height - h; y < c.height; ++y)
                            pixel(x, y, .85, .56, .16);
                }
        }
        return Results{b};
    }

    return {};
}
} // namespace touch::ops
