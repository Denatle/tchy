#include <SDL3/SDL.h>
#include <iostream>
#include <touch/core.hpp>
int main(int argc, char** argv) {
    try {
        SDL_Init(SDL_INIT_AUDIO);
        touch::FFmpegMedia media;
        touch::Registry registry;
        touch::register_builtins(registry, media);
        touch::PluginManager plugins(registry);
        bool benchmark = argc > 2 && std::string(argv[2]) == "--benchmark";
        if (argc > 2 && !benchmark)
            plugins.load(argv[2]);
        touch::JsonProjects projects;
        touch::Graph g;
        if (argc > 1)
            g = projects.load(argv[1], registry);
        else {
            auto a = touch::add(g, *registry.find("builtin.Pattern"), 0, 0);
            auto b = touch::add(g, *registry.find("builtin.VideoOut"), 200, 0);
            touch::connect(g, {{a, 0}, {b, 0}}, registry);
        }
        touch::Evaluator engine(registry);
        touch::Monitor m;
        auto start = std::chrono::steady_clock::now();
        if (benchmark) {
            auto first = engine.tick(g, {g.width, g.height, 0, 0, true});
            if (!first.errors.empty())
                throw std::runtime_error(first.errors.begin()->second);
            std::cout
                << "Cold first frame: "
                << std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count()
                << "s\n";
            start = std::chrono::steady_clock::now();
        }
        int frames = 0;
        for (int tick = 0; tick < (benchmark ? 100 : 1); ++tick) {
            uint64_t frame = uint64_t(tick) * g.fps / 100;
            bool due = tick == 0 || frame != uint64_t(tick - 1) * g.fps / 100;
            m = engine.tick(g, {g.width, g.height, frame, tick * .01, due});
            if (due && m.frame)
                ++frames;
        }
        if (benchmark) {
            double seconds =
                std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            std::cout << "100 audio ticks / " << frames << " video frames in " << seconds << "s ("
                      << frames / seconds << " video FPS)\n";
        }

        for (auto [id, error] : m.errors)
            std::cerr << id << ": " << error << '\n';
        std::cout << registry.all().size() << " operators; " << g.nodes.size() << " nodes; frame "
                  << (m.frame
                          ? std::to_string(m.frame->width) + "x" + std::to_string(m.frame->height)
                          : "none")
                  << '\n';
        return m.errors.empty() ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
