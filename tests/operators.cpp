#include <iostream>
#include <touch/core.hpp>
#define REQUIRE(x)                                                                                 \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error("Failed: " #x);                                               \
    } while (false)
using namespace touch;
namespace {
Value fixture(Signal signal) {
    if (signal == Signal::Video)
        return data(signal, 8, 4, 4, .5f);
    if (signal == Signal::Audio)
        return data(signal, 480, 1, 2, .25f);
    if (signal == Signal::Control)
        return data(signal, 1, 1, 1, .5f);
    auto d = std::const_pointer_cast<Data>(data(signal, 8, 4, 1, .5f));
    if (signal == Signal::Table)
        d->rows = {{"value", "0.5"}};
    d->names = {"a", "b", "c", "d"};
    return d;
}
class FakeSource : public MediaSource {
  public:
    std::pair<Value, Value> read(double) override {
        return {fixture(Signal::Video), fixture(Signal::Audio)};
    }
};
class FakeMedia : public MediaFactory {
  public:
    std::unique_ptr<MediaSource> open(const Node&, const Context&) override {
        return std::make_unique<FakeSource>();
    }
};
} // namespace
int main() {
    try {
        FakeMedia media;
        Registry registry;
        register_builtins(registry, media);
        for (auto& [id, d] : registry.all()) {
            try {
                Graph g;
                auto nid = add(g, d, 0, 0);
                Node n = *g.find(nid);
                std::vector<Value> inputs;
                for (auto signal : d.inputs)
                    inputs.push_back(fixture(signal));
                inputs.resize(inputs.size() + d.params.size());
                if (d.legacy.starts_with("Resize"))
                    n.params = {8, 4};
                if (d.legacy == "Wavetable")
                    inputs[1] = data(Signal::Channels, 1, 4, 1, 1);
                auto op = d.create();
                auto output = op->process(n, inputs, {8, 4, 1, 0, true});
                REQUIRE(output.size() == (d.outputs.empty() ? 1 : d.outputs.size()));
                for (size_t i = 0; i < d.outputs.size(); ++i) {
                    REQUIRE(output[i]);
                    REQUIRE(output[i]->type == d.outputs[i]);
                    for (auto x : output[i]->values)
                        REQUIRE(std::isfinite(x));
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(id + ": " + e.what());
            }
        }
        Graph g;
        auto id = add(g, *registry.find("builtin.Blend"), 0, 0);
        auto a = std::const_pointer_cast<Data>(data(Signal::Video, 1, 1, 4));
        a->values = {1, 0, 0, 0};
        auto b = std::const_pointer_cast<Data>(data(Signal::Video, 1, 1, 4));
        b->values = {0, 0, 1, 1};
        auto blended =
            registry.find("builtin.Blend")->create()->process(*g.find(id), {a, b}, {}).at(0);
        REQUIRE(blended->values[0] == 0);
        REQUIRE(blended->values[2] == 1);
        REQUIRE(blended->values[3] == .5f);
        auto scan = registry.find("builtin.ScanOnce");
        g = {};
        id = add(g, *scan, 0, 0);
        auto player = scan->create();
        auto table = data(Signal::Channels, 3, 1, 1, 1);
        auto trigger = data(Signal::Control, 1, 1, 1, 1);
        auto result = player->process(*g.find(id), {table, trigger}, {}).at(0);
        REQUIRE(result->values[0] > .1);
        REQUIRE(result->values[3] == 0);
        result = player->process(*g.find(id), {table, trigger}, {}).at(0);
        REQUIRE(result->values[0] == 0);
        std::cout << "All 77 operator contracts, alpha blending and scan trigger passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
