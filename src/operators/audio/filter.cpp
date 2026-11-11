#include "types.hpp"
namespace touch::ops {
Results filter(const std::string& kind, Oscillator& state, const std::vector<Value>& inputs,
               const Parameters& p) {
    auto input = require(inputs, 0);
    auto out = copy(input);
    if (state.filter.size() != size_t(input->components))
        state.filter.assign(input->components, 0);
    float alpha = 1 - std::exp(-tau * p(0) / 48000), gain = std::pow(10, p(0) / 20);
    for (size_t i = 0; i < out->values.size(); ++i) {
        if (kind == "Gain")
            out->values[i] *= gain;
        else {
            float& last = state.filter[i % input->components];
            last += alpha * (out->values[i] - last);
            out->values[i] = last;
        }
    }
    return {out};
}
} // namespace touch::ops
