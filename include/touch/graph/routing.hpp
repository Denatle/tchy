#pragma once
#include <touch/graph/graph.hpp>
namespace touch {
Signal port_signal(const Graph&, const Registry&, Port, bool output);
void insert_on_edge(Graph&, Edge, Id, const Registry&);
void erase_and_reconnect(Graph&, const std::set<Id>&, const Registry&);
std::vector<const Descriptor*> converters(const Registry&, Signal, Signal);
} // namespace touch
