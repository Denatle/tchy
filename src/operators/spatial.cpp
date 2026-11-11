#include "common.hpp"
namespace touch::ops {
Maybe spatial(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
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
    if (k.starts_with("Resize"))
        return Results{resize(a, int(std::lround(p(0))), int(std::lround(p(1))), choice(0) == 0)};
    if (k.starts_with("Crop") || k.starts_with("Extend") || k == "Transform") {
        int w = a->width, h = a->height, ox = 0, oy = 0;
        if (k.starts_with("Crop")) {
            ox = int(std::lround(p(0)));
            oy = int(std::lround(p(1)));
            if (ox >= w || oy >= h)
                throw std::runtime_error("Crop outside image");
            w = p(2) == 0 ? w - ox : std::min(w - ox, int(std::lround(p(2))));
            h = p(3) == 0 ? h - oy : std::min(h - oy, int(std::lround(p(3))));
        }
        if (k.starts_with("Extend")) {
            ox = -int(std::lround(p(0)));
            oy = -int(std::lround(p(2)));
            w += int(p(0) + p(1));
            h += int(p(2) + p(3));
        }
        if (w > 4096 || h > 4096)
            throw std::runtime_error("Image exceeds 4096 pixels");
        auto b = make(a->type, w, h, a->components);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                float sx = x + ox, sy = y + oy;
                if (k == "Transform") {
                    float cx = (w - 1) * .5f, cy = (h - 1) * .5f, dx = x - cx - p(0) * w,
                          dy = y - cy - p(1) * h, angle = p(3) * tau / 360;
                    sx = (dx * std::cos(angle) + dy * std::sin(angle)) / p(2) + cx;
                    sy = (dy * std::cos(angle) - dx * std::sin(angle)) / p(2) + cy;
                }
                auto v = sample(a, sx, sy, choice(0), k == "Transform");
                std::copy_n(v.begin(), b->components,
                            b->values.begin() + (y * w + x) * b->components);
            }
        return Results{b};
    }

    return {};
}
} // namespace touch::ops
