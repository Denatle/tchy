#pragma once
#include "theme.hpp"
#include <optional>
#include <touch/operators/types.hpp>
namespace touch::ui {
struct CanvasRect{ImVec2 min,max;bool contains(ImVec2)const;bool overlaps(CanvasRect)const;};
bool reroute(const Node&);
ImVec2 node_size(const Node&,const Descriptor*);
std::vector<ImVec2> cable_points(ImVec2,ImVec2);
float segment_distance(ImVec2,ImVec2,ImVec2);
std::optional<ImVec2> intersection(ImVec2,ImVec2,ImVec2,ImVec2);
std::optional<size_t> compatible_port(const Descriptor&,Signal,bool output);
std::string port_label(const Descriptor&,bool,size_t);
}
