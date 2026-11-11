#include <touch/editor/document.hpp>
namespace touch {
void Document::erase() {
    edit([&](Graph& g) {
        std::erase_if(g.nodes, [&](auto& n) { return selected.contains(n.id); });
        std::erase_if(g.edges, [&](auto e) {
            return selected.contains(e.from.node) || selected.contains(e.to.node);
        });
        for (auto id : selected)
            g.layout.erase(id);
        for (auto p : {&g.preview, &g.video, &g.audio})
            if (selected.contains(*p))
                *p = 0;
    });
    selected.clear();
}
void Document::duplicate() {
    edit([&](Graph& g) {
        std::map<Id, Id> remap;
        auto ns = g.nodes;
        auto es = g.edges;
        for (auto n : ns)
            if (selected.contains(n.id)) {
                Id old = n.id;
                n.id = ++g.next;
                remap[old] = n.id;
                g.nodes.push_back(n);
                auto l = g.layout[old];
                l.x += 32;
                l.y += 32;
                g.layout[n.id] = l;
            }
        for (auto e : es)
            if (remap.contains(e.from.node) && remap.contains(e.to.node))
                g.edges.push_back(
                    {{remap[e.from.node], e.from.index}, {remap[e.to.node], e.to.index}});
        selected.clear();
        for (auto [a, b] : remap)
            selected.insert(b);
    });
}
} // namespace touch
