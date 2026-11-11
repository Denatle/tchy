#pragma once
#include <functional>
#include <touch/graph/types.hpp>
namespace touch {
struct Context {
    int width = 1280, height = 720;
    uint64_t frame = 0;
    double seconds = 0;
    bool video_due = true;
    Id inspect = 0;
};
class Operator {
  public:
    virtual ~Operator() = default;
    virtual std::vector<Value> process(const Node&, const std::vector<Value>&, const Context&) = 0;
};
struct Descriptor {
    std::string id, label, legacy;
    std::vector<Signal> inputs, outputs;
    std::vector<std::string> input_keys, output_keys;
    std::vector<Parameter> params;
    std::vector<Choice> choices;
    bool video_rate = false, cacheable = false;
    std::function<std::unique_ptr<Operator>()> create;
};

} // namespace touch
