#include <touch/engine/plan.hpp>
namespace touch {
std::vector<EvaluationStep> compile_plan(const Graph& graph, Id inspect) {
    std::vector<Id> roots = {graph.preview, graph.video, graph.audio};
    auto active = order(graph, roots);
    if (inspect && graph.find(inspect)) {
        bool safe = true;
        for (auto id : order(graph, {inspect})) {
            auto node = graph.find(id);
            if ((node->type == "builtin.Camera" || node->type == "builtin.Microphone") &&
                std::find(active.begin(), active.end(), id) == active.end())
                safe = false;
        }
        if (safe)
            roots.push_back(inspect);
    }
    auto ordered = order(graph, roots);
    std::map<Id, std::vector<Edge>> incoming;
    for (auto edge : graph.edges)
        incoming[edge.to.node].push_back(edge);
    std::vector<EvaluationStep> plan;
    for (auto id : ordered)
        plan.push_back({id, std::move(incoming[id])});
    return plan;
}
} // namespace touch
