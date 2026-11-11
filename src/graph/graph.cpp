#include <touch/graph/graph.hpp>
namespace touch {
std::vector<Id> order(const Graph& g, const std::vector<Id>& roots) {
    std::set<Id> present;
    std::map<Id, std::vector<Id>> incoming;
    for (auto& n : g.nodes)
        present.insert(n.id);
    for (auto edge : g.edges)
        incoming[edge.to.node].push_back(edge.from.node);
    std::map<Id, int> marks;
    std::vector<Id> result;
    std::vector<std::pair<Id, bool>> stack;
    for (auto root : roots) {
        if (!root)
            continue;
        stack.push_back({root, false});
        while (!stack.empty()) {
            auto [id, expanded] = stack.back();
            stack.pop_back();
            if (!present.contains(id))
                throw std::runtime_error("Missing node");
            if (expanded) {
                marks[id] = 2;
                result.push_back(id);
                continue;
            }
            if (marks[id] == 2)
                continue;
            if (marks[id] == 1)
                throw std::runtime_error("Graph contains a cycle");
            marks[id] = 1;
            stack.push_back({id, true});
            auto& deps = incoming[id];
            for (auto it = deps.rbegin(); it != deps.rend(); ++it)
                stack.push_back({*it, false});
        }
    }
    return result;
}
static std::optional<Signal> signal(const Graph& g, const Registry& r, Port p, bool output) {
    auto n = g.find(p.node);
    auto d = n ? r.find(n->type) : nullptr;
    if (!d)
        return {};
    auto& ports = output ? d->outputs : d->inputs;
    if (p.index < ports.size())
        return ports[p.index];
    if (!output && p.index - ports.size() < d->params.size())
        return Signal::Control;
    return {};
}
void validate(const Graph& g, const Registry& r) {
    if (g.width < 1 || g.height < 1 || g.width > 4096 || g.height > 4096 || g.fps < 1 ||
        g.fps > 240)
        throw std::runtime_error("Invalid video settings");
    std::vector<Id> ids;
    std::set<Id> used;
    for (auto& n : g.nodes) {
        if (!n.id || n.id == UINT64_MAX || !used.insert(n.id).second)
            throw std::runtime_error("Invalid node ID");
        ids.push_back(n.id);
        if (auto d = r.find(n.type)) {
            if (n.params.size() != d->params.size() || n.choices.size() > d->choices.size())
                throw std::runtime_error("Invalid parameters: " + n.type);
            for (size_t i = 0; i < n.params.size(); ++i)
                if (!std::isfinite(n.params[i]) || n.params[i] < d->params[i].min ||
                    n.params[i] > d->params[i].max)
                    throw std::runtime_error("Parameter out of range");
            for (size_t i = 0; i < n.choices.size(); ++i)
                if (n.choices[i] < 0 || size_t(n.choices[i]) >= d->choices[i].items.size())
                    throw std::runtime_error("Invalid choice");
        }
        if (auto i = g.layout.find(n.id); i != g.layout.end()) {
            auto l = i->second;
            if (!std::isfinite(l.x) || !std::isfinite(l.y) || !std::isfinite(l.width) ||
                l.width < 112 || l.width > 640)
                throw std::runtime_error("Invalid layout");
        }
    }
    std::set<Port> inputs;
    for (auto e : g.edges) {
        if (!g.find(e.from.node) || !g.find(e.to.node) || !inputs.insert(e.to).second)
            throw std::runtime_error("Invalid connection");
        auto a = signal(g, r, e.from, true), b = signal(g, r, e.to, false);
        if (r.find(g.find(e.from.node)->type) && r.find(g.find(e.to.node)->type) && (!a || a != b))
            throw std::runtime_error("Incompatible ports");
    }
    for (auto id : {g.video, g.audio, g.preview})
        if (id && !g.find(id))
            throw std::runtime_error("Missing output");
    order(g, ids);
}
void connect(Graph& g, Edge e, const Registry& r) {
    auto a = signal(g, r, e.from, true), b = signal(g, r, e.to, false);
    if (!a || a != b)
        throw std::runtime_error("Incompatible ports");
    auto old = g.edges;
    std::erase_if(g.edges, [&](auto x) { return x.to == e.to; });
    g.edges.push_back(e);
    try {
        validate(g, r);
    } catch (...) {
        g.edges = old;
        throw;
    }
}
Id add(Graph& g, const Descriptor& d, float x, float y) {
    for (auto& n : g.nodes)
        g.next = std::max(g.next, n.id);
    if (g.next >= UINT64_MAX - 1)
        throw std::runtime_error("ID exhausted");
    Node n;
    n.id = ++g.next;
    n.type = d.id;
    n.input_keys = d.input_keys;
    n.output_keys = d.output_keys;
    for (auto p : d.params)
        n.params.push_back(p.initial);
    n.table = {{"value", "0.5"}};
    g.nodes.push_back(n);
    g.layout[n.id] = {x, y, 164};
    if (d.legacy == "VideoOut" && !g.video)
        g.video = n.id;
    if (d.legacy == "AudioOut" && !g.audio)
        g.audio = n.id;
    return n.id;
}

} // namespace touch
