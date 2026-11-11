#include <touch/engine/runtime.hpp>
namespace touch {
Monitor Evaluator::tick(const Graph& g, const Context& c) {
    Monitor m;
    std::vector<Id> nodes, roots = {g.preview, g.video, g.audio, c.inspect};
    for (auto& n : g.nodes)
        nodes.push_back(n.id);
    if (!plan_valid || nodes != planned_nodes || roots != planned_roots ||
        g.edges != planned_edges) {
        plan = compile_plan(g, c.inspect);
        planned_nodes = nodes;
        planned_roots = roots;
        planned_edges = g.edges;
        plan_valid = true;
    }
    std::set<Id> active;
    for (auto& step : plan)
        active.insert(step.node);
    std::erase_if(instances, [&](auto& item) { return !active.contains(item.first); });
    std::erase_if(caches, [&](auto& item) { return !active.contains(item.first); });
    for (auto& step : plan) {
        auto id = step.node;
        auto n = g.find(id);
        auto d = registry.find(n->type);
        if (!d) {
            m.errors[id] = "Missing plugin: " + n->type;
            continue;
        }
        bool blocked = false;
        std::vector<Value> inputs(d->inputs.size() + d->params.size());
        for (auto e : step.incoming) {
            if (m.errors.contains(e.from.node)) {
                blocked = true;
                break;
            }
            auto it = m.values.find(e.from.node);
            if (it != m.values.end() && e.from.index < it->second.size() &&
                e.to.index < inputs.size())
                inputs[e.to.index] = it->second[e.from.index];
        }
        if (blocked) {
            m.errors[id] = "Upstream node unavailable";
            continue;
        }
        try {
            if (n->params.size() != d->params.size())
                throw std::runtime_error("Saved parameters do not match installed operator");
            for (size_t i = 0; i < n->choices.size(); ++i)
                if (i >= d->choices.size() || n->choices[i] < 0 ||
                    size_t(n->choices[i]) >= d->choices[i].items.size())
                    throw std::runtime_error("Saved choices do not match installed operator");
            if (!instances.contains(id) || types[id] != n->type) {
                instances[id] = d->create();
                types[id] = n->type;
                caches.erase(id);
            }
            auto& cache = caches[id];
            bool matches = cache.inputs == inputs && cache.params == n->params &&
                           cache.choices == n->choices && cache.file == n->file &&
                           cache.names == n->names && cache.rows == n->table;
            if ((d->video_rate && !c.video_due && !cache.outputs.empty()) ||
                (d->cacheable && matches && !cache.outputs.empty())) {
                m.values[id] = cache.outputs;
            } else {
                m.values[id] = instances[id]->process(*n, inputs, c);
                cache = {inputs, m.values[id], n->params, n->choices, n->file, n->names, n->table};
            }
        } catch (const std::exception& e) {
            caches.erase(id);
            m.errors[id] = e.what();
        }
    }
    auto output = [&](Id id) -> Value {
        auto it = m.values.find(id);
        return it != m.values.end() && !it->second.empty() ? it->second[0] : nullptr;
    };
    m.frame = output(g.preview ? g.preview : g.video);
    m.audio = output(g.audio);
    if (m.frame && m.frame->type == Signal::Mask) {
        auto image =
            std::const_pointer_cast<Data>(data(Signal::Video, m.frame->width, m.frame->height, 4));
        for (int i = 0; i < image->width * image->height; ++i) {
            for (int c = 0; c < 3; ++c)
                image->values[i * 4 + c] = m.frame->values[i];
            image->values[i * 4 + 3] = 1;
        }
        m.frame = image;
    }
    return m;
}
} // namespace touch
