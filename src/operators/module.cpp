#include "common.hpp"
#include "factory.hpp"
namespace touch {
using namespace ops;
void register_builtins(Registry& r, MediaFactory& media) {
    auto define = [&](std::string k, std::string label, std::vector<Signal> inputs,
                      std::vector<Signal> outputs, std::vector<Parameter> params) {
        Descriptor d;
        d.id = "builtin." + k;
        d.label = label;
        d.legacy = k;
        d.inputs = inputs;
        d.outputs = outputs;
        d.params = params;
        auto choices = [&](std::string name, std::vector<std::string> items) {
            d.choices.push_back({name, items});
        };
        if (k == "ExtractChannel")
            choices("Channel", {"Luma", "Red", "Green", "Blue", "Alpha"});
        if (k == "ImageChannels")
            choices("Components", {"Luma", "Red", "Green", "Blue", "Alpha", "RGBA"});
        if (k == "ChannelsImage")
            choices("Layout", {"Gray rows", "RGBA groups"});
        if (k == "Transform" || k.starts_with("Extend"))
            choices("Boundary", {"Transparent", "Edge", "Repeat", "Mirror"});
        if (k.starts_with("Resize"))
            choices("Filter", {"Bilinear", "Nearest"});
        if (k == "ImageFormat" || k == "ChannelFormat")
            choices("Format", {"32-bit float", "16-bit float", "8-bit UNORM"});
        if (k == "SetAlpha")
            choices("Mode", {"Replace", "Multiply"});
        if (k == "ReduceChannels") {
            choices("Axis", {"Samples", "Channels"});
            choices("Operation", {"Mean", "Min", "Max"});
        }
        if (k == "Wavetable")
            choices("DC", {"Remove", "Keep"});
        if (k == "StereoMix")
            choices("Mode", {"Average", "Pair"});
        bool audio = std::set<std::string>{"Tone",      "Lfo",         "Gain",      "LowPass",
                                           "Mix",       "ControlTone", "TableTone", "VideoScan",
                                           "Wavetable", "ScanOnce",    "StereoMix"}
                         .contains(k);
        bool media_node =
            std::set<std::string>{"Image", "Movie", "Camera", "Microphone", "AudioFile"}.contains(
                k);
        d.video_rate =
            !audio && std::find(outputs.begin(), outputs.end(), Signal::Video) != outputs.end();
        d.cacheable = !audio && k != "Grain" && k != "Movie" && k != "Camera" &&
                      k != "Microphone" && k != "AudioFile";
        d.create = builtin_factory(d, media);
        r.add(std::move(d));
    };
#include "catalog.inc"
}
} // namespace touch
