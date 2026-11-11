#include "types.hpp"
namespace touch::ops {
Results mixer(const std::string& kind, const Node& node, const std::vector<Value>& inputs,
              const Parameters& p) {
    auto a = require(inputs, 0);
    if (kind == "Mix") {
        auto b = require(inputs, 1);
        same(a, b);
        if (a->components != b->components)
            throw std::runtime_error("Audio channel counts differ");
        auto out = copy(a);
        for (size_t i = 0; i < out->values.size(); ++i)
            out->values[i] = a->values[i] * p(0) + b->values[i] * p(1);
        return {out};
    }
    auto out = make(Signal::Audio, 480, 1, 2);
    out->names = {"left", "right"};
    for (int i = 0; i < 480; ++i)
        for (int ch = 0; ch < 2; ++ch) {
            float value = 0;
            if (node.choice(0) == 1) {
                int index = int(p(ch)) - 1;
                if (index < 0 || index >= a->components)
                    throw std::runtime_error("Audio channel missing");
                value = a->values[i * a->components + index];
            } else {
                for (int j = 0; j < a->components; ++j)
                    value += a->values[i * a->components + j];
                value /= a->components;
            }
            out->values[i * 2 + ch] = value;
        }
    return {out};
}
} // namespace touch::ops
