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
        touch::Registry r;
        touch::register_builtins(r, media);
        REQUIRE(r.all().size() == 77);
        touch::Graph g;
        auto a = touch::add(g, *r.find("builtin.Constant"), 0, 0),
             b = touch::add(g, *r.find("builtin.ControlMath"), 200, 0);
        touch::connect(g, {{a, 0}, {b, 0}}, r);
        auto old = g.edges;
        bool failed = false;
        try {
            touch::connect(g, {{b, 0}, {b, 0}}, r);
        } catch (...) {
            failed = true;
        }
        REQUIRE(failed && g.edges == old);
        g.preview = b;
        touch::Evaluator evaluator(r);
        auto m = evaluator.tick(g, {});
        REQUIRE(m.errors.empty());
        REQUIRE(m.frame && m.frame->values[0] == .5f);
        touch::PluginManager plugins(r);
        plugins.load(argv[1]);
        auto p = touch::add(g, *r.find("example.constant"), 0, 0);
        g.preview = p;
        m = evaluator.tick(g, {});
        REQUIRE(m.frame && m.frame->values[0] == .5f);
        if (argc > 2) {
            touch::JsonProjects projects;
            auto scene = projects.load(argv[2], r);
            REQUIRE(scene.nodes.size() == 23);
            m = evaluator.tick(scene, {});
            for (auto [id, e] : m.errors)
                std::cerr << id << ": " << e << '\n';
            REQUIRE(m.errors.empty());
            REQUIRE(m.frame && m.frame->width == 1280);
            auto path = std::filesystem::temp_directory_path() / "touch-rewrite-test.json";
            projects.save(path, scene);
            auto roundtrip = projects.load(path, r);
            REQUIRE(roundtrip.nodes.size() == scene.nodes.size());
            REQUIRE(roundtrip.edges == scene.edges);
            std::filesystem::remove(path);
        }
        std::cout << (argc > 2 ? "Core, plugin and legacy scene checks passed\n"
                               : "Core and plugin checks passed (no scene fixture)\n");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
