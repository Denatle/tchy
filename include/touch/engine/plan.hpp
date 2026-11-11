#pragma once
#include <touch/graph/graph.hpp>
namespace touch {
struct EvaluationStep {
    Id node;
    std::vector<Edge> incoming;
};
std::vector<EvaluationStep> compile_plan(const Graph&, Id inspect);
} // namespace touch
