#include "editor.hpp"
namespace touch::ui {
ImVec2 Editor::screen(float x, float y) const {
    return {origin.x + pan.x + x * zoom, origin.y + pan.y + y * zoom};
}
ImVec2 Editor::world(ImVec2 p) const {
    return {(p.x - origin.x - pan.x) / zoom, (p.y - origin.y - pan.y) / zoom};
}
CanvasRect Editor::node_bounds(Id id)const{auto n=document.graph.find(id);if(!n)return {};auto l=document.graph.layout.at(id);auto size=node_size(*n,services.registry.find(n->type));if(!reroute(*n))size.x=l.width;auto at=screen(l.x,l.y);at={std::round(at.x),std::round(at.y)};return {at,{at.x+std::round(size.x*zoom),at.y+std::round(size.y*zoom)}};}
ImVec2 Editor::port_position(Port p,bool output)const{auto n=document.graph.find(p.node);if(!n)return {};auto bounds=node_bounds(p.node);float y=reroute(*n)?7:theme::header+theme::row*(p.index+.5f);return {output?bounds.max.x:bounds.min.x,std::round(bounds.min.y+y*zoom)};}
void Editor::connect_endpoints(Endpoint a, Endpoint b) {
    if (a.output == b.output)
        return;
    auto from = a.output ? a.port : b.port, to = a.output ? b.port : a.port;
    try {
        auto source = port_signal(document.graph, services.registry, from, true),
             target = port_signal(document.graph, services.registry, to, false);
        if (source != target) {
            conversion = Edge{from, to};
            return;
        }
        document.edit([&](Graph& g) {connect(g,{from,to},services.registry);if(wire_original&&wire_original->to!=to)std::erase(g.edges,*wire_original);});
        changed();
    } catch (const std::exception& e) {
        status = e.what();
    }
}
} // namespace touch::ui
