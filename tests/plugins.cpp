#include <iostream>
#include <touch/core.hpp>
#define REQUIRE(x)                                                                                 \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error("Failed: " #x);                                               \
    } while (false)
int main(int argc, char** argv) {
    try {
        touch::FFmpegMedia media;
        touch::Registry registry;
        touch::register_builtins(registry, media);
        touch::PluginManager plugins(registry);
        bool rejected = false;
        try {
            plugins.load(argv[2]);
        } catch (...) {
            rejected = true;
        }
        REQUIRE(rejected && registry.all().size() == 77);
        plugins.load(argv[1]);
        rejected = false;
        try {
            plugins.load(argv[1]);
        } catch (...) {
            rejected = true;
        }
        REQUIRE(rejected && registry.all().size() == 79);
        touch::Graph g;
        auto source = touch::add(g, *registry.find("builtin.Constant"), 0, 0),
             target = touch::add(g, *registry.find("example.constant"), 0, 0);
        g.find(source)->params[0] = .8f;
        touch::connect(g, {{source, 0}, {target, 0}}, registry);
        g.preview = target;
        touch::Evaluator evaluator(registry);
        auto m = evaluator.tick(g, {});
        REQUIRE(m.frame && std::abs(m.frame->values[0] - .8f) < .00001f);
        rejected = false;
        try {
            plugins.unload_all(true);
        } catch (...) {
            rejected = true;
        }
        REQUIRE(rejected && registry.find("example.constant"));
        evaluator.reset();
        auto descriptor = registry.find("example.constant");
        auto instance = descriptor->create();
        touch::Node node = *g.find(target);
        plugins.unload_all(false);
        auto output = instance->process(node, {}, {});
        REQUIRE(output.size() == 1 && output[0]->values[0] == .5f);
        instance.reset();
        plugins.load(argv[1]);
        REQUIRE(registry.find("example.invert"));
        std::cout
            << "ABI rejection, duplicate registration, modulation and library lifetimes passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
