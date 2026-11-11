#include "types.hpp"
namespace touch::ops {
Results oscillator(const std::string& kind, Oscillator& state, const Node&,
                   const std::vector<Value>& inputs,const Context& context, const Parameters& p) {
    if (kind == "Lfo") {
        if(state.control_time){double elapsed=context.seconds-*state.control_time;if(elapsed<0)state.control_phase=0;else state.control_phase=std::fmod(state.control_phase+p(0)*elapsed,1.0);}
        state.control_time=context.seconds;
        auto out = make(Signal::Control, 1, 1);
        out->values[0] = (std::sin(state.control_phase * tau) + 1) * .5f;
        return {out};
    }
    auto out = make(Signal::Audio, 480, 1, 2);
    out->names = {"left", "right"};
    float frequency = p(0), gain = p(1);
    Value image;
    int row = 0;
    float mean = 0;
    if (kind == "ControlTone")
        gain *= inputs.empty() || !inputs[0] ? 0 : clamp(inputs[0]->values[0]);
    if (kind == "TableTone")
        gain *= table_value(inputs.empty() ? nullptr : inputs[0]);
    if (kind == "VideoScan") {
        image = require(inputs, 0);
        row = std::lround(p(0) * (image->height - 1));
        frequency = p(1);
        gain = p(2);
        for (int x = 0; x < image->width; ++x)
            mean += luma((image->values.data() + ((row * image->width + x) * 4)));
        mean /= image->width;
    }
    for (int i = 0; i < 480; ++i) {
        float value = std::sin(state.phase * tau);
        if (image) {
            float position = state.phase * (image->width - 1);
            int a = int(position), b = std::min(a + 1, image->width - 1);
            float first = luma((image->values.data() + ((row * image->width + a) * 4)));
            float second = luma((image->values.data() + ((row * image->width + b) * 4)));
            value = std::clamp(first + (second - first) * (position - a) - mean, -1.f, 1.f);
        }
        out->values[i * 2] = out->values[i * 2 + 1] = value * gain;
        state.phase = std::fmod(state.phase + frequency / 48000, 1.f);
    }
    return {out};
}
} // namespace touch::ops
