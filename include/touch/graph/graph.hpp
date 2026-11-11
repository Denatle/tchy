#pragma once
#include <algorithm>
#include <optional>
#include <set>
#include <touch/operators/registry.hpp>
namespace touch {
std::vector<Id> order(const Graph&, const std::vector<Id>&);
void validate(const Graph&, const Registry&);
void connect(Graph&, Edge, const Registry&);
Id add(Graph&, const Descriptor&, float x, float y);

} // namespace touch
