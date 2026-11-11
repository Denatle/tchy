#include "factory.hpp"
#include "common.hpp"
namespace touch {
using namespace ops;
namespace {
class FunctionOperator : public Operator {
    std::function<std::vector<Value>(const Node&, const std::vector<Value>&, const Context&)> fn;

  public:
    explicit FunctionOperator(decltype(fn) f) : fn(std::move(f)) {}
    std::vector<Value> process(const Node& n, const std::vector<Value>& v,
                               const Context& c) override {
        return fn(n, v, c);
    }
};
Results process(const std::string& k, const Node& n, const std::vector<Value>& in, const Context& c,
                const Parameters& p) {
    if (k.starts_with("Reroute") || k.ends_with("Null") || k.ends_with("Out"))
        return {in.empty() ? nullptr : in[0]};
    if (k.ends_with("Switch")) {
        size_t i = p(0) >= .5f ? 1 : 0;
        return {i < in.size() ? in[i] : nullptr};
    }
    for (auto family : {control, generators, channels, masks, spatial, effects})
        if (auto result = family(k, n, in, c, p))
            return *result;
    throw std::runtime_error("Connect the required inputs for " + k);
}
class MediaOperator : public Operator {
    MediaFactory& factory;
    std::string file;
    std::unique_ptr<MediaSource> source;
    std::optional<std::string> failure;
    bool attempted = false;

  public:
    explicit MediaOperator(MediaFactory& f) : factory(f) {}
    std::vector<Value> process(const Node& n, const std::vector<Value>&,
                               const Context& c) override {
        if (!attempted || file != n.file) {
            source.reset();
            failure.reset();
            file = n.file;
            attempted = true;
            try {
                source = factory.open(n, c);
            } catch (const std::exception& e) {
                failure = e.what();
            }
        }
        if (failure)
            throw std::runtime_error(*failure);
        auto [video, audio] = source->read(c.seconds);
        if (n.type == "builtin.Microphone" || n.type == "builtin.AudioFile")
            return {audio};
        if (n.type == "builtin.Movie")
            return {video, audio};
        return {video};
    }
};
} // namespace
std::function<std::unique_ptr<Operator>()> builtin_factory(const Descriptor& descriptor,
                                                           MediaFactory& media) {
    auto k = descriptor.legacy;
    auto inputs = descriptor.inputs;
    auto params = descriptor.params;
    std::function<std::unique_ptr<Operator>()> result;
    bool audio = std::set<std::string>{"Tone",      "Lfo",         "Gain",      "LowPass",
                                       "Mix",       "ControlTone", "TableTone", "VideoScan",
                                       "Wavetable", "ScanOnce",    "StereoMix"}
                     .contains(k);
    bool media_node =
        std::set<std::string>{"Image", "Movie", "Camera", "Microphone", "AudioFile"}.contains(k);
    if (media_node)
        result = [&media] { return std::make_unique<MediaOperator>(media); };
    else if (audio)
        result = [k, inputs, params] {
            return std::make_unique<FunctionOperator>(
                [k, inputs, params, state = Oscillator{}](
                    const Node& n, const std::vector<Value>& in, const Context& c) mutable {
                    Parameters p;
                    for (size_t i = 0; i < params.size(); ++i) {
                        size_t port = inputs.size() + i;
                        p.values.push_back(port < in.size() && in[port]
                                               ? params[i].min + clamp(in[port]->values[0]) *
                                                                     (params[i].max - params[i].min)
                                               : n.params.at(i));
                    }
                    return audio_process(k, state, n, in, c, p);
                });
        };
    else
        result = [k, inputs, params] {
            return std::make_unique<FunctionOperator>(
                [k, inputs, params](const Node& n, const std::vector<Value>& in, const Context& c) {
                    Parameters p;
                    for (size_t i = 0; i < params.size(); ++i) {
                        size_t port = inputs.size() + i;
                        p.values.push_back(port < in.size() && in[port]
                                               ? params[i].min + clamp(in[port]->values[0]) *
                                                                     (params[i].max - params[i].min)
                                               : n.params.at(i));
                    }
                    return process(k, n, in, c, p);
                });
        };
    return result;
}
} // namespace touch
