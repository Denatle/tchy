#include "common.hpp"
namespace touch::ops {
Maybe effects(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
              const Parameters& p) {
    auto get = [&](size_t i) -> Value {
        if (i >= in.size() || !in[i])
            throw std::runtime_error("Connect input " + std::to_string(i + 1));
        return in[i];
    };
    auto choice = [&](int i) { return n.choice(i); };
    if (in.empty() || !in[0])
        return {};
    auto a = in[0];
    auto b = copy(a);
    if (k == "ImageFormat" || k == "ChannelFormat") {
        b->format = choice(0);
        b->values.pack(choice(0));
        return Results{b};
    }
    if (k == "Blur") {
        int radius = std::lround(p(0));
        if (!radius)
            return Results{a};
        for (int i = 0; i < a->width * a->height; ++i)
            for (int j = 0; j < 3; ++j)
                b->values[i * 4 + j] *= b->values[i * 4 + 3];
        for (int axis = 0; axis < 2; ++axis) {
            auto src = b->values;
            int len = axis ? a->height : a->width, lines = axis ? a->width : a->height;
            for (int line = 0; line < lines; ++line)
                for (int ch = 0; ch < 4; ++ch) {
                    auto index = [&](int pos) {
                        pos = std::clamp(pos, 0, len - 1);
                        return (axis ? pos * a->width + line : line * a->width + pos) * 4 + ch;
                    };
                    float sum = 0;
                    for (int j = -radius; j <= radius; ++j)
                        sum += src[index(j)];
                    for (int x = 0; x < len; ++x) {
                        b->values[index(x)] = std::max(0.f, sum / (radius * 2 + 1));
                        sum += src[index(x + radius + 1)] - src[index(x - radius)];
                    }
                }
        }
        for (int i = 0; i < a->width * a->height; ++i)
            if (b->values[i * 4 + 3] > 0)
                for (int j = 0; j < 3; ++j)
                    b->values[i * 4 + j] /= b->values[i * 4 + 3];
        return Results{b};
    }
    if (k == "Blend" || k == "MaskedMix" || k == "SetAlpha" || k == "Over") {
        auto d = get(1);
        same(a, d);
        Value mask = k == "MaskedMix" ? get(2) : nullptr;
        if (mask)
            same(a, mask);
        for (int i = 0; i < a->width * a->height; ++i) {
            auto dest = (b->values.data() + (i * 4));
            auto first = (a->values.data() + (i * 4));
            if (k == "SetAlpha")
                dest[3] = d->values[i] * (choice(0) == 1 ? first[3] : 1);
            else if (k == "Over") {
                auto second = (d->values.data() + (i * 4));
                float alpha = first[3] + (1 - first[3]) * second[3];
                for (int j = 0; j < 3; ++j)
                    dest[j] =
                        alpha > 0
                            ? (first[j] * first[3] + (1 - first[3]) * second[j] * second[3]) / alpha
                            : 0;
                dest[3] = alpha;
            } else
                mix_pixel(dest, first, (d->values.data() + (i * 4)),
                          p(0) * (mask ? mask->values[i] : 1));
        }
        return Results{b};
    }
    uint32_t rng = uint32_t(c.frame) + 1;
    for (size_t i = 0; i < b->values.size(); ++i) {
        float& x = b->values[i];
        if (k == "Levels" && i % 4 < 3)
            x = clamp((x - .5f) * p(1) + .5f + p(0));
        else if (k == "Grain") {
            if (i % 4 == 0) {
                rng ^= rng << 13;
                rng ^= rng >> 17;
                rng ^= rng << 5;
            }
            if (i % 4 < 3)
                x = clamp(x + ((rng & 255) / 255.f - .5f) * p(0));
        } else if (k == "Threshold")
            x = x >= p(0) ? 1 : 0;
        else if (k == "InvertMask")
            x = 1 - x;
        else if (k == "MaskRange" || k == "ChannelRange")
            x = p(2) + (p(3) - p(2)) * (p(0) == p(1) ? 0 : clamp((x - p(0)) / (p(1) - p(0))));
    }
    if (k == "Levels" || k == "Grain" || k == "Threshold" || k == "InvertMask" ||
        k == "MaskRange" || k == "ChannelRange")
        return Results{b};
    return {};
}
} // namespace touch::ops
