#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <touch/data/samples.hpp>
#include <touch/json.hpp>
#include <tuple>
#include <vector>
namespace touch {
using Id = uint64_t;
enum class Signal { Video = 1, Audio, Control, Table, Mask, Channels };
struct Data {
    Signal type;
    int width = 1, height = 1, components = 1, format = 0;
    Samples values;
    std::vector<std::string> names;
    std::vector<std::array<std::string, 2>> rows;
};
using Value = std::shared_ptr<const Data>;
Value data(Signal type, int w, int h, int c = 1, float fill = 0);
struct Parameter {
    std::string name;
    float initial, min, max;
};
struct Choice {
    std::string name;
    std::vector<std::string> items;
};
struct Node {
    Id id = 0;
    std::string type;
    std::vector<std::string> input_keys, output_keys;
    std::vector<float> params;
    std::vector<int> choices;
    std::string file, names;
    std::vector<std::array<std::string, 2>> table;
    Json extra;
    bool operator==(const Node&)const=default;
    int choice(size_t i) const {
        return i < choices.size() ? choices[i] : 0;
    }
};
struct Layout {
    float x = 0, y = 0, width = 164;
    bool operator==(const Layout&)const=default;
};
struct Port {
    Id node = 0;
    size_t index = 0;
    bool operator==(const Port&) const = default;
    bool operator<(const Port& b) const {
        return std::tie(node, index) < std::tie(b.node, b.index);
    }
};
struct Edge {
    Port from, to;
    bool operator==(const Edge&) const = default;
};
struct Graph {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::map<Id, Layout> layout;
    Id preview = 0, video = 0, audio = 0, next = 0;
    int width = 1280, height = 720, fps = 30;
    bool operator==(const Graph&)const=default;
    Node* find(Id);
    const Node* find(Id) const;
};

} // namespace touch
