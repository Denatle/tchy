#include "types.hpp"
namespace touch::ops {
Results playback(const std::string& kind, Oscillator& state, const Node& node,
                 const std::vector<Value>& inputs, const Parameters& p) {
    auto table = require(inputs, 0);
    int count = table->height;
    auto out = make(Signal::Audio, 480, 1, count);
    out->names = table->names;
    if (kind == "ScanOnce") {
        bool trigger = inputs.size() > 1 && inputs[1] && inputs[1]->values[0] >= .5f;
        if (trigger && !state.trigger) {
            state.scan = table;
            state.position = 0;
        }
        state.trigger = trigger;
        if (state.scan) {
            out = make(Signal::Audio, 480, 1, state.scan->height);
            out->names = state.scan->names;
            for (int i = 0; i < 480; ++i)
                for (int ch = 0; ch < state.scan->height; ++ch)
                    if (state.position + i < state.scan->width)
                        out->values[i * state.scan->height + ch] =
                            state.scan->values[ch * state.scan->width + state.position + i] * p(0);
            state.position += 480;
            if (state.position >= state.scan->width)
                state.scan.reset();
        }
        return {out};
    }
    Value weights = inputs.size() > 1 ? inputs[1] : nullptr;
    if (weights && (weights->height != count || weights->width != 1))
        throw std::runtime_error("Wavetable needs one weight per channel");
    std::vector<float> means(count, 0);
    if (node.choice(0) == 0)
        for (int ch = 0; ch < count; ++ch)
            for (int x = 0; x < table->width; ++x)
                means[ch] += table->values[ch * table->width + x] / table->width;
    for (int i = 0; i < 480; ++i) {
        float position = state.phase * table->width;
        int a = int(position) % table->width, b = (a + 1) % table->width;
        for (int ch = 0; ch < count; ++ch) {
            float first = table->values[ch * table->width + a],
                  second = table->values[ch * table->width + b];
            out->values[i * count + ch] = (first + (second - first) * (position - a) - means[ch]) *
                                          p(1) * (weights ? weights->values[ch] : 1);
        }
        state.phase = std::fmod(state.phase + p(0) / 48000, 1.f);
    }
    return {out};
}
} // namespace touch::ops
