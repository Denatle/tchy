#include "common.hpp"
namespace touch::ops {
Maybe control(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
              const Parameters& p) {
    auto get = [&](size_t i) -> Value {
        if (i >= in.size() || !in[i])
            throw std::runtime_error("Connect input " + std::to_string(i + 1));
        return in[i];
    };
    auto choice = [&](int i) { return n.choice(i); };
    if (k == "Constant" || k == "ControlMath" || k == "FromTable" || k == "Luma" || k == "Rms" ||
        k == "Peak") {
        auto b = make(Signal::Control, 1, 1);
        float v = 0;
        if (k == "Constant")
            v = p(0);
        else if (k == "ControlMath")
            v = get(0)->values[0] * p(0) + p(1);
        else if (k == "FromTable")
            v = table_value(get(0));
        else {
            auto a = get(0);
            if (k == "Luma") {
                for (size_t i = 0; i < a->values.size(); i += 4)
                    v += luma((a->values.data() + (i)));
                v /= a->width * a->height;
            } else {
                for (float x : a->values)
                    v = k == "Peak" ? std::max(v, std::abs(x)) : v + x * x;
                if (k == "Rms")
                    v = std::sqrt(v / a->values.size());
            }
        }
        b->values[0] = clamp(v);
        return Results{b};
    }
    if (k == "Table" || k == "ToTable" || k == "AudioStats" || k == "VideoStats") {
        auto b = make(Signal::Table, 1, 1);
        if (k == "Table")
            b->rows = n.table;
        else if (k == "ToTable")
            b->rows = {{"value", std::to_string(get(0)->values[0])}};
        else if (k == "AudioStats") {
            auto a = get(0);
            float rms = 0, peak = 0;
            for (float x : a->values) {
                rms += x * x;
                peak = std::max(peak, std::abs(x));
            }
            rms = clamp(std::sqrt(rms / a->values.size()));
            b->rows = {{"value", std::to_string(rms)},
                       {"rms", std::to_string(rms)},
                       {"peak", std::to_string(clamp(peak))}};
        } else {
            auto a = get(0);
            float rgb[4] = {};
            for (size_t i = 0; i < a->values.size(); i += 32)
                for (int j = 0; j < 3; ++j)
                    rgb[j] += a->values[i + j];
            float count = std::ceil(a->values.size() / 32.);
            for (int j = 0; j < 3; ++j)
                rgb[j] /= count;
            b->rows = {{"value", std::to_string(luma(rgb))},
                       {"luma", std::to_string(luma(rgb))},
                       {"red", std::to_string(rgb[0])},
                       {"green", std::to_string(rgb[1])},
                       {"blue", std::to_string(rgb[2])}};
        }
        return Results{b};
    }

    return {};
}
} // namespace touch::ops
