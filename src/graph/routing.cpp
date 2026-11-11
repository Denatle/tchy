#include <touch/graph/routing.hpp>
namespace touch {
Signal port_signal(const Graph& g, const Registry& r, Port p, bool output) {
    auto n = g.find(p.node);
    auto d = n ? r.find(n->type) : nullptr;
    if (!d)
        throw std::runtime_error("Missing operator");
    if (output) {
        if (p.index >= d->outputs.size())
            throw std::runtime_error("Missing output");
        return d->outputs[p.index];
    }
    if (p.index < d->inputs.size())
        return d->inputs[p.index];
    if (p.index < d->inputs.size() + d->params.size())
        return Signal::Control;
    throw std::runtime_error("Missing input");
}
void insert_on_edge(Graph& g, Edge edge, Id id, const Registry& r) {
    auto n = g.find(id);
    auto d = n ? r.find(n->type) : nullptr;
    if (!d)
        throw std::runtime_error("Missing operator");
    auto signal = port_signal(g, r, edge.from, true);
    auto input = std::find(d->inputs.begin(), d->inputs.end(), signal),
         output = std::find(d->outputs.begin(), d->outputs.end(), signal);
    if (input == d->inputs.end() || output == d->outputs.end())
        throw std::runtime_error("Operator cannot be inserted on this wire");
    Graph candidate = g;
    std::erase(candidate.edges, edge);
    connect(candidate, {edge.from, {id, size_t(input - d->inputs.begin())}}, r);
    connect(candidate, {{id, size_t(output - d->outputs.begin())}, edge.to}, r);
    g = std::move(candidate);
}
void erase_and_reconnect(Graph& g, const std::set<Id>& selected, const Registry& r) {
    auto candidate = g;
    std::vector<Edge> links;
    for (auto outgoing : g.edges)
        if (selected.contains(outgoing.from.node) && !selected.contains(outgoing.to.node))
            for (auto incoming : g.edges)
                if (incoming.to.node == outgoing.from.node &&
                    !selected.contains(incoming.from.node)) {
                    try {
                        if (port_signal(g, r, incoming.from, true) ==
                            port_signal(g, r, outgoing.to, false)) {
                            links.push_back({incoming.from, outgoing.to});
                            break;
                        }
                    } catch (...) {
                    }
                }
    std::erase_if(candidate.nodes, [&](auto n) { return selected.contains(n.id); });
    std::erase_if(candidate.edges, [&](auto e) {
        return selected.contains(e.from.node) || selected.contains(e.to.node);
    });
    for (auto id : selected)
        candidate.layout.erase(id);
    for (auto root : {&candidate.preview, &candidate.video, &candidate.audio})
        if (selected.contains(*root))
            *root = 0;
    for (auto edge : links)
        connect(candidate, edge, r);
    g = std::move(candidate);
}
std::vector<const Descriptor*> converters(const Registry& r, Signal from, Signal to) {
    std::vector<const Descriptor*> results;
    for (auto& [id, d] : r.all())
        if(from!=to&&d.legacy!="SeparateRgba"&&(d.inputs.size()==1||d.legacy=="Wavetable"||d.legacy=="ScanOnce")&&d.inputs[0]==from&&std::find(d.outputs.begin(),d.outputs.end(),to)!=d.outputs.end())
            results.push_back(&d);
    return results;
}
} // namespace touch
