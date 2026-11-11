#include "audio/types.hpp"
namespace touch::ops {
Value require(const std::vector<Value>& inputs, size_t index) {
    if (index >= inputs.size() || !inputs[index])
        throw std::runtime_error("Connect input " + std::to_string(index + 1));
    return inputs[index];
}
Results audio_process(const std::string& kind, Oscillator& state, const Node& node,
                      const std::vector<Value>& inputs, const Context& context, const Parameters& p) {
    if (kind == "Gain" || kind == "LowPass")
        return filter(kind, state, inputs, p);
    if (kind == "Mix" || kind == "StereoMix")
        return mixer(kind, node, inputs, p);
    if (kind == "Wavetable" || kind == "ScanOnce")
        return playback(kind, state, node, inputs, p);
    return oscillator(kind, state, node, inputs, context,p);
}
} // namespace touch::ops
