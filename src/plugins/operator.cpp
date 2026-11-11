#include <algorithm>
#include <touch/json.hpp>
#include <touch/operators/types.hpp>
#include <touch/plugin.h>
namespace touch {
namespace {
class PluginOperator : public Operator {
    std::shared_ptr<void> library;
    TouchOperatorApi api;
    void* instance;
    std::vector<Value> outputs;
    std::vector<Parameter> parameters;
    size_t input_count;
    std::vector<Signal> output_signals;
    static float* allocate(void* ctx, uint32_t port, uint32_t signal, uint32_t w, uint32_t h,
                           uint32_t components, uint32_t format) {
        auto self = static_cast<PluginOperator*>(ctx);
        try {
            if (port >= self->outputs.size() || signal < 1 || signal > 6 || format > 2 ||
                Signal(signal) != self->output_signals[port] || (signal == 1 && components != 4) ||
                ((signal == 3 || signal == 4 || signal == 5 || signal == 6) && components != 1) ||
                (signal == 2 && (w != 480 || h != 1)))
                return nullptr;
            auto d = std::const_pointer_cast<Data>(data(Signal(signal), w, h, components));
            d->format = format;
            self->outputs[port] = d;
            return d->values.data();
        } catch (...) {
            return nullptr;
        }
    }
    static int metadata(void* ctx, uint32_t port, const char* text) {
        auto self = static_cast<PluginOperator*>(ctx);
        try {
            if (port >= self->outputs.size() || !self->outputs[port])
                return -1;
            auto d = std::const_pointer_cast<Data>(self->outputs[port]);
            auto j = Json::parse(text);
            d->names.clear();
            d->rows.clear();
            for (size_t i = 0; i < j["names"].size(); ++i)
                d->names.push_back(j["names"][i].str());
            for (size_t i = 0; i < j["rows"].size(); ++i)
                d->rows.push_back({j["rows"][i][size_t(0)].str(), j["rows"][i][size_t(1)].str()});
            return 0;
        } catch (...) {
            return -1;
        }
    }

  public:
    PluginOperator(std::shared_ptr<void> lib, TouchOperatorApi a, size_t count,
                   std::vector<Parameter> params, size_t inputs, std::vector<Signal> signals)
        : library(std::move(lib)), api(a), instance(a.create()), outputs(count),
          parameters(std::move(params)), input_count(inputs), output_signals(std::move(signals)) {
        if (!instance)
            throw std::runtime_error("Plugin create failed");
    }
    ~PluginOperator() override {
        api.destroy(instance);
    }
    std::vector<Value> process(const Node& n, const std::vector<Value>& inputs,
                               const Context& c) override {
        std::fill(outputs.begin(), outputs.end(), nullptr);
        std::vector<TouchValue> values;
        std::vector<std::string> meta;
        meta.reserve(inputs.size());
        for (auto a : inputs) {
            if (!a) {
                values.push_back({});
                continue;
            }
            Json j, ns = Json::array(), rows = Json::array();
            for (auto& name : a->names)
                ns.push(name);
            for (auto row : a->rows) {
                auto pair = Json::array();
                pair.push(row[0]);
                pair.push(row[1]);
                rows.push(pair);
            }
            j.set("names", ns);
            j.set("rows", rows);
            meta.push_back(j.dump());
            values.push_back({uint32_t(a->type), uint32_t(a->width), uint32_t(a->height),
                              uint32_t(a->components), uint32_t(a->format), a->values.data(),
                              a->values.size(), meta.back().c_str()});
        }
        std::vector<float> resolved = n.params;
        for (size_t i = 0; i < parameters.size(); ++i) {
            size_t port = input_count + i;
            if (port < inputs.size() && inputs[port])
                resolved[i] = parameters[i].min + std::clamp(inputs[port]->values[0], 0.f, 1.f) *
                                                      (parameters[i].max - parameters[i].min);
        }
        Json node = Json::parse(n.extra.dump());
        node.set("file", n.file);
        node.set("channel_names", n.names);
        auto rows = Json::array();
        for (auto row : n.table) {
            auto pair = Json::array();
            pair.push(row[0]);
            pair.push(row[1]);
            rows.push(pair);
        }
        node.set("table", rows);
        auto text = node.dump();
        TouchHost host{TOUCH_ABI_VERSION, sizeof(TouchHost), this, allocate, metadata};
        TouchProcess p{sizeof(TouchProcess),
                       uint32_t(c.width),
                       uint32_t(c.height),
                       c.frame,
                       c.seconds,
                       resolved.data(),
                       resolved.size(),
                       n.choices.data(),
                       n.choices.size(),
                       values.data(),
                       values.size(),
                       text.c_str()};
        char error[1024] = {};
        int result = api.process(instance, &host, &p, error, sizeof(error));
        error[sizeof(error) - 1] = 0;
        if (result != 0)
            throw std::runtime_error(error[0] ? error : "Plugin process failed");
        for (auto value : outputs)
            if (value)
                std::const_pointer_cast<Data>(value)->values.pack(value->format);
        return outputs;
    }
};
} // namespace
std::unique_ptr<Operator> plugin_operator(std::shared_ptr<void> lib, TouchOperatorApi api,
                                          size_t count, std::vector<Parameter> parameters,
                                          size_t inputs, std::vector<Signal> signals) {
    return std::make_unique<PluginOperator>(std::move(lib), api, count, std::move(parameters),
                                            inputs, std::move(signals));
}
} // namespace touch
