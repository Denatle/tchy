#include "common.hpp"
namespace touch::ops {
Maybe masks(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
            const Parameters& p) {
    auto get = [&](size_t i) -> Value {
        if (i >= in.size() || !in[i])
            throw std::runtime_error("Connect input " + std::to_string(i + 1));
        return in[i];
    };
    auto choice = [&](int i) { return n.choice(i); };
    if (k == "CombineRgba") {
        Value first;
        for (auto a : in)
            if (a) {
                first = a;
                break;
            }
        if (!first)
            throw std::runtime_error("Connect RGBA component");
        auto b = make(Signal::Video, first->width, first->height, 4);
        for (int j = 0; j < 4; ++j) {
            Value a = size_t(j) < in.size() ? in[j] : nullptr;
            if (a)
                same(first, a);
            for (int i = 0; i < b->width * b->height; ++i)
                b->values[i * 4 + j] = a ? a->values[i] : (j == 3 ? 1 : 0);
        }
        return Results{b};
    }
    if (in.empty() || !in[0])
        return {};
    auto a = in[0];
    if (k == "ExtractChannel" || k == "SeparateRgba") {
        std::vector<Value> out;
        int count = k == "SeparateRgba" ? 4 : 1;
        for (int j = 0; j < count; ++j) {
            int ch = count == 4 ? j + 1 : choice(0);
            auto b = make(Signal::Mask, a->width, a->height);
            for (int i = 0; i < a->width * a->height; ++i)
                b->values[i] =
                    clamp(ch == 0 ? luma((a->values.data() + (i * 4))) : a->values[i * 4 + ch - 1]);
            out.push_back(b);
        }
        return Results(out);
    }
    if (k == "MaskImage") {
        auto b = make(Signal::Video, a->width, a->height, 4);
        for (int i = 0; i < a->width * a->height; ++i) {
            for (int j = 0; j < 3; ++j)
                b->values[i * 4 + j] = a->values[i];
            b->values[i * 4 + 3] = 1;
        }
        return Results{b};
    }
    return {};
}
} // namespace touch::ops
