#include <iostream>
#include <touch/core.hpp>
namespace {
struct Counts {
    std::atomic<int> open = 0, writes = 0;
};
class Output : public touch::AudioOutput {
    Counts& counts;

  public:
    explicit Output(Counts& c) : counts(c) {
        ++counts.open;
    }
    ~Output() override {
        --counts.open;
    }
    void write(touch::Value, float) override {
        ++counts.writes;
    }
};
class Outputs : public touch::AudioOutputs {
    Counts& counts;

  public:
    explicit Outputs(Counts& c) : counts(c) {}
    std::unique_ptr<touch::AudioOutput> open() override {
        return std::make_unique<Output>(counts);
    }
};
} // namespace
int main(int argc, char** argv) {
    try {
        touch::FFmpegMedia media;
        touch::Registry registry;
        touch::register_builtins(registry, media);
        touch::PluginManager plugins(registry);
        Counts counts;
        Outputs outputs(counts);
        touch::SteadyClock clock;
        touch::Runtime runtime(registry, outputs, clock);
        touch::Graph graph;
        auto tone = touch::add(graph, *registry.find("builtin.Tone"), 0, 0),
             out = touch::add(graph, *registry.find("builtin.AudioOut"), 0, 0);
        touch::connect(graph, {{tone, 0}, {out, 0}}, registry);
        runtime.update(graph);
        runtime.mute(false);
        runtime.run(true);
        auto deadline = clock.now() + std::chrono::seconds(2);
        while (!counts.writes && clock.now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (!counts.writes)
            throw std::runtime_error("No audio output");
        plugins.load(argv[1]);
        auto constant = touch::add(graph, *registry.find("example.constant"), 0, 0);
        graph.preview = constant;
        runtime.update(graph);
        bool seen = false;
        while (clock.now() < deadline) {
            auto m = runtime.snapshot();
            if (m.frame && m.frame->type == touch::Signal::Control) {
                seen = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        if (!seen)
            throw std::runtime_error("Runtime did not execute newly loaded plugin");
        runtime.run(false);
        if (counts.open)
            throw std::runtime_error("Stop did not release device");
        plugins.unload_all(false);
        if (registry.find("example.constant"))
            throw std::runtime_error("Plugin still registered");
        plugins.load(argv[1]);
        runtime.run(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        runtime.run(false);
        if (counts.open)
            throw std::runtime_error("Restart leaks audio output");
        std::cout << "Live plugin loading and synchronous stop/restart passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
