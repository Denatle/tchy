#include <iostream>
#include <touch/core.hpp>
#include <touch/graph/routing.hpp>
#define REQUIRE(x)                                                                                 \
    do {                                                                                           \
        if (!(x))                                                                                  \
            throw std::runtime_error("Failed: " #x);                                               \
    } while (false)
int main() {
    try {
        touch::FFmpegMedia media;
        touch::Registry registry;
        touch::register_builtins(registry, media);
        touch::Document doc;
        auto a = touch::add(doc.graph, *registry.find("builtin.Pattern"), 0, 0),
             b = touch::add(doc.graph, *registry.find("builtin.VideoOut"), 300, 0),
             c = touch::add(doc.graph, *registry.find("builtin.VideoNull"), 150, 0);
        touch::Edge edge{{a, 0}, {b, 0}};
        touch::connect(doc.graph, edge, registry);
        doc.edit([&](touch::Graph& g) { touch::insert_on_edge(g, edge, c, registry); });
        REQUIRE(doc.graph.edges.size() == 2);
        doc.undo();
        REQUIRE(doc.graph.edges.size() == 1);
        doc.redo();
        REQUIRE(doc.graph.edges.size() == 2);
        doc.edit([&](touch::Graph& g) { touch::erase_and_reconnect(g, {c}, registry); });
        REQUIRE(doc.graph.edges.size() == 1 && doc.graph.edges[0] == edge);
        doc.undo();
        REQUIRE(doc.graph.find(c));
        auto missing = doc.graph.find(c);
        missing->type = "vendor.missing";
        missing->extra.set("plugin_state", "preserved");
        auto encoded = touch::encode_project(doc.graph, "/tmp/project.json");
        auto loaded = touch::decode_project(encoded, "/tmp/project.json", registry);
        REQUIRE(loaded.find(c)->extra["plugin_state"].str() == "preserved");
        REQUIRE(loaded.edges == doc.graph.edges);
        touch::Evaluator engine(registry);
        auto result = engine.tick(loaded, {});
        REQUIRE(result.errors.contains(c));
        REQUIRE(!result.frame);
        doc.graph.next = 100;
        auto roundtrip = touch::decode_project(
            touch::encode_project(doc.graph, "/tmp/project.json"), "/tmp/project.json", registry);
        REQUIRE(roundtrip.next == 100);
        auto next = touch::add(roundtrip, *registry.find("builtin.Constant"), 0, 0);
        REQUIRE(next == 101);
        touch::Registry reordered;
        auto d = *registry.find("builtin.Blend");
        d.input_keys = {"second", "first", "param.Mix"};
        reordered.add(d);
        touch::Graph stable;
        auto from = touch::add(stable, *registry.find("builtin.Blend"), 0, 0);
        auto to = touch::add(stable, *registry.find("builtin.Blend"), 0, 0);
        stable.edges.push_back({{from, 0}, {to, 0}});
        auto source = touch::encode_project(stable, "/tmp/project.json");
        auto old_keys = source["nodes"][size_t(0)];
        auto keys = touch::Json::array();
        keys.push("first");
        keys.push("second");
        keys.push("param.Mix");
        old_keys.set("input_keys", keys);
        source["nodes"][size_t(1)].set("input_keys", keys);
        source["edges"][size_t(0)]["to"].set("key", "first");
        auto migrated = touch::decode_project(source, "/tmp/project.json", reordered);
        REQUIRE(migrated.edges[0].to.index == 1);
        std::cout << "Routing transactions, missing plugins and stable port keys passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
