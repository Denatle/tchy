#pragma once
#include <touch/graph/graph.hpp>
namespace touch {
class Document {
  public:
    Graph graph;
    std::set<Id> selected;
    std::vector<Graph> undo_stack, redo_stack;
    void edit(const std::function<void(Graph&)>& f) {
        auto old = graph;
        try {
            f(graph);
        } catch (...) {
            graph = old;
            throw;
        }
        commit(std::move(old));
    }
    void commit(Graph before){if(graph==before)return;undo_stack.push_back(std::move(before));if(undo_stack.size()>100)undo_stack.erase(undo_stack.begin());redo_stack.clear();}
    void prune_selection(){std::erase_if(selected,[&](Id id){return !graph.find(id);});}
    void undo() {
        if (undo_stack.empty())
            return;
        redo_stack.push_back(graph);
        graph = undo_stack.back();
        undo_stack.pop_back();
        prune_selection();
    }
    void redo() {
        if (redo_stack.empty())
            return;
        undo_stack.push_back(graph);
        graph = redo_stack.back();
        redo_stack.pop_back();
        prune_selection();
    }
    void erase();
    void duplicate();
};
} // namespace touch
