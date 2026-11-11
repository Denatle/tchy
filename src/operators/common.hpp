#pragma once
#include <algorithm>
#include <cmath>
#include <numbers>
#include <numeric>
#include <optional>
#include <touch/operators/types.hpp>
namespace touch::ops {
inline constexpr float tau = 2 * std::numbers::pi_v<float>;
using Mutable = std::shared_ptr<Data>;
inline Mutable make(Signal t, int w, int h, int c = 1, float fill = 0) {
    return std::const_pointer_cast<Data>(data(t, w, h, c, fill));
}
inline Mutable copy(Value a) {
    if (!a)
        throw std::runtime_error("Connect an input");
    auto result = std::make_shared<Data>(*a);
    result->format = 0;
    return result;
}
inline float clamp(float x) {
    return std::clamp(x, 0.f, 1.f);
}
inline float luma(const float* p) {
    return .2126f * p[0] + .7152f * p[1] + .0722f * p[2];
}
inline void same(Value a, Value b) {
    if (!a || !b || a->width != b->width || a->height != b->height)
        throw std::runtime_error("Inputs must have matching dimensions");
}
inline int boundary(int x, int size, int mode) {
    if (x >= 0 && x < size)
        return x;
    if (mode == 0)
        return x < 0 || x >= size ? -1 : x;
    if (mode == 1)
        return std::clamp(x, 0, size - 1);
    int n = mode == 2 ? size : size * 2;
    int v = (x % n + n) % n;
    return mode == 2 ? v : std::min(v, n - 1 - v);
}
inline std::array<float, 4> sample(Value a, float x, float y, int mode, bool linear) {
    std::array<float, 4> out{};
    int ix = linear ? int(std::floor(x)) : int(std::round(x)),
        iy = linear ? int(std::floor(y)) : int(std::round(y));
    for (int j = 0; j < (linear ? 2 : 1); ++j)
        for (int i = 0; i < (linear ? 2 : 1); ++i) {
            int sx = boundary(ix + i, a->width, mode), sy = boundary(iy + j, a->height, mode);
            if (sx < 0 || sy < 0)
                continue;
            float weight = linear ? (i ? x - ix : 1 - x + ix) * (j ? y - iy : 1 - y + iy) : 1;
            auto p = (a->values.data() + ((sy * a->width + sx) * a->components));
            for (int k = 0; k < a->components; ++k)
                out[k] += p[k] * weight * (a->components == 4 && k < 3 ? p[3] : 1);
        }
    if (a->components == 4 && out[3] > 0)
        for (int k = 0; k < 3; ++k)
            out[k] /= out[3];
    return out;
}
inline Mutable resize(Value a, int w, int h, bool linear) {
    auto b = make(a->type, w, h, a->components);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            auto p = sample(a, (x + .5f) * a->width / w - .5f, (y + .5f) * a->height / h - .5f, 1,
                            linear);
            std::copy_n(p.begin(), a->components, b->values.begin() + (y * w + x) * a->components);
        }
    return b;
}
inline void mix_pixel(float* d, const float* a, const float* b, float t) {
    float alpha = a[3] * (1 - t) + b[3] * t;
    for (int k = 0; k < 3; ++k)
        d[k] = alpha > 0 ? (a[k] * a[3] * (1 - t) + b[k] * b[3] * t) / alpha : 0;
    d[3] = alpha;
}
inline float table_value(Value a) {
    if (a)
        for (auto row : a->rows)
            if (row[0] == "value") {
                try {
                    float value = std::stof(row[1]);
                    return std::isfinite(value) ? clamp(value) : 0;
                } catch (...) {
                }
            }
    return 0;
}

struct Parameters {
    std::vector<float> values;
    float operator()(size_t i) const {
        return values[i];
    }
};
using Results = std::vector<Value>;
using Maybe = std::optional<Results>;
Maybe control(const std::string&, const Node&, const std::vector<Value>&, const Context&,
              const Parameters&);
Maybe generators(const std::string&, const Node&, const std::vector<Value>&, const Context&,
                 const Parameters&);
Maybe channels(const std::string&, const Node&, const std::vector<Value>&, const Context&,
               const Parameters&);
Maybe spatial(const std::string&, const Node&, const std::vector<Value>&, const Context&,
              const Parameters&);
Maybe effects(const std::string&, const Node&, const std::vector<Value>&, const Context&,
              const Parameters&);
struct Oscillator {
    float phase = 0;
    double control_phase=0;
    std::optional<double> control_time;
    std::vector<float> filter;
    bool trigger = false;
    Value scan;
    int position = 0;
};
Results audio_process(const std::string&, Oscillator&, const Node&, const std::vector<Value>&,
                      const Context&, const Parameters&);
} // namespace touch::ops
namespace touch::ops {
Maybe masks(const std::string&, const Node&, const std::vector<Value>&, const Context&,
            const Parameters&);
}
