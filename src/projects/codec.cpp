#include <touch/projects/repository.hpp>
namespace touch {
static Json port_json(Port p, const Graph& g, bool output) {
    Json j;
    j.set("node", Json(p.node));
    j.set("index", Json(uint64_t(p.index)));
    if (auto n = g.find(p.node)) {
        const auto& keys = output ? n->output_keys : n->input_keys;
        if (p.index < keys.size())
            j.set("key", keys[p.index]);
    }
    return j;
}
Graph decode_project(const Json& j, const std::filesystem::path& path, const Registry& r) {
    int version = j["version"].number();
    if (version != 1 && version != 2)
        throw std::runtime_error("Unsupported project version");
    Graph g;
    auto ns = j["nodes"];
    for (size_t i = 0; i < ns.size(); ++i) {
        auto v = ns[i];
        Node n;
        n.extra = v;
        n.id = v["id"].id();
        n.type = v[version == 1 ? "kind" : "type"].str();
        if (version == 1) {
            bool found = false;
            for (auto& [id, d] : r.all())
                if (d.legacy == n.type) {
                    n.type = id;
                    found = true;
                    break;
                }
            if (!found)
                n.type = "legacy." + n.type;
        }
        for (size_t k = 0; k < v["params"].size(); ++k)
            n.params.push_back(v["params"][k].number());
        for (size_t k = 0; k < v["choices"].size(); ++k)
            n.choices.push_back(v["choices"][k].number());
        for (size_t k = 0; k < v["input_keys"].size(); ++k)
            n.input_keys.push_back(v["input_keys"][k].str());
        for (size_t k = 0; k < v["output_keys"].size(); ++k)
            n.output_keys.push_back(v["output_keys"][k].str());
        if (auto descriptor = r.find(n.type)) {
            n.input_keys = descriptor->input_keys;
            n.output_keys = descriptor->output_keys;
        }
        n.names = v["channel_names"].str();
        n.file = v["file"].str();
        if (!n.file.empty() && std::filesystem::path(n.file).is_relative())
            n.file = (std::filesystem::absolute(path).parent_path() / n.file)
                         .lexically_normal()
                         .string();
        for (size_t k = 0; k < v["table"].size(); ++k)
            n.table.push_back({v["table"][k][size_t(0)].str(), v["table"][k][size_t(1)].str()});
        auto l = version == 1 ? v : v["layout"];
        g.layout[n.id] = {float(l["x"].number()), float(l["y"].number()),
                          l.has("width") ? float(l["width"].number()) : 164};
        g.next = std::max(g.next, n.id);
        g.nodes.push_back(std::move(n));
    }
    g.next = std::max({g.next, j["next_id"].id(), j["last_id"].id()});
    if (g.next == UINT64_MAX)
        throw std::runtime_error("Invalid ID counter");
    auto es = j["edges"];
    for (size_t i = 0; i < es.size(); ++i) {
        auto e = es[i];
        auto read_port = [&](const char* name, bool output) {
            auto p = e[name];
            Port result{p["node"].id(), size_t(p["index"].id())};
            if (p.has("key")) {
                auto n = g.find(result.node);
                if (!n)
                    throw std::runtime_error("Missing node");
                auto& keys = output ? n->output_keys : n->input_keys;
                auto it = std::find(keys.begin(), keys.end(), p["key"].str());
                if (it == keys.end())
                    throw std::runtime_error("Missing port key: " + p["key"].str());
                result.index = it - keys.begin();
            }
            return result;
        };
        g.edges.push_back({read_port("from", true), read_port("to", false)});
    }
    g.preview = j["preview"].id();
    g.video = j["video"].id();
    g.audio = j["audio"].id();
    if (version == 1) {
        auto vs = j["video_settings"];
        auto res = vs["resolution"].str();
        if (res == "P480") {
            g.width = 854;
            g.height = 480;
        }
        if (res == "P1080") {
            g.width = 1920;
            g.height = 1080;
        }
        auto fps = vs["rate"].str();
        if (fps == "Fps24")
            g.fps = 24;
        if (fps == "Fps60")
            g.fps = 60;
    } else {
        g.width = j["width"].number();
        g.height = j["height"].number();
        g.fps = j["fps"].number();
    }
    validate(g, r);
    return g;
}
Json encode_project(const Graph& g, const std::filesystem::path& path) {
    Json j;
    j.set("version", 2);
    j.set("next_id", Json(g.next));
    Json ns = Json::array();
    for (auto& n : g.nodes) {
        auto v = Json::parse(n.extra.dump());
        for (auto key : {"kind", "x", "y", "width"})
            v.erase(key);
        v.set("id", Json(n.id));
        v.set("type", n.type);
        Json ik = Json::array(), ok = Json::array();
        for (auto k : n.input_keys)
            ik.push(k);
        for (auto k : n.output_keys)
            ok.push(k);
        v.set("input_keys", ik);
        v.set("output_keys", ok);
        Json ps = Json::array(), cs = Json::array(), ts = Json::array();
        for (auto p : n.params)
            ps.push(Json(double(p)));
        for (auto c : n.choices)
            cs.push(c);
        for (auto row : n.table) {
            auto t = Json::array();
            t.push(row[0]);
            t.push(row[1]);
            ts.push(t);
        }
        v.set("params", ps);
        v.set("choices", cs);
        v.set("table", ts);
        v.set("channel_names", n.names);
        auto file = std::filesystem::path(n.file);
        if (!file.empty())
            file = file.lexically_relative(std::filesystem::absolute(path).parent_path());
        v.set("file", file.string());
        auto l = g.layout.at(n.id);
        Json layout;
        layout.set("x", Json(double(l.x)));
        layout.set("y", Json(double(l.y)));
        layout.set("width", Json(double(l.width)));
        v.set("layout", layout);
        ns.push(v);
    }
    j.set("nodes", ns);
    Json es = Json::array();
    for (auto e : g.edges) {
        Json v;
        v.set("from", port_json(e.from, g, true));
        v.set("to", port_json(e.to, g, false));
        es.push(v);
    }
    j.set("edges", es);
    j.set("preview", Json(g.preview));
    j.set("video", Json(g.video));
    j.set("audio", Json(g.audio));
    j.set("width", g.width);
    j.set("height", g.height);
    j.set("fps", g.fps);
    return j;
}
} // namespace touch
