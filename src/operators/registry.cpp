#include <touch/operators/registry.hpp>
namespace touch {
namespace {
void normalize(Descriptor& d) {
    if (d.id.empty() || !d.create)
        throw std::runtime_error("Invalid operator identifier or factory");
    for (auto& p : d.params)
        if (p.name.empty() || !std::isfinite(p.initial) || !std::isfinite(p.min) ||
            !std::isfinite(p.max) || p.min > p.max || p.initial < p.min || p.initial > p.max)
            throw std::runtime_error("Invalid parameter descriptor");
    for (auto& c : d.choices)
        if (c.name.empty() || c.items.empty())
            throw std::runtime_error("Invalid choice descriptor");
    if (d.input_keys.empty()) {
        for (size_t i = 0; i < d.inputs.size(); ++i)
            d.input_keys.push_back("input." + std::to_string(i));
        for (auto& p : d.params)
            d.input_keys.push_back("param." + p.name);
    }
    if (d.output_keys.empty())
        for (size_t i = 0; i < d.outputs.size(); ++i)
            d.output_keys.push_back("output." + std::to_string(i));
    if (d.input_keys.size() != d.inputs.size() + d.params.size() ||
        d.output_keys.size() != d.outputs.size())
        throw std::runtime_error("Invalid port keys");
    for (auto keys : {d.input_keys, d.output_keys}) {
        std::set<std::string> unique;
        for (auto key : keys)
            if (key.empty() || !unique.insert(key).second)
                throw std::runtime_error("Duplicate port key");
    }
}
} // namespace
void Registry::add(Descriptor d) {
    std::vector<Descriptor> pending;
    pending.push_back(std::move(d));
    add_batch(std::move(pending));
}
void Registry::add_batch(std::vector<Descriptor> descriptors) {
    std::map<std::string, Descriptor> pending;
    for (auto& d : descriptors) {
        normalize(d);
        auto id = d.id;
        if (pending.contains(id))
            throw std::runtime_error("Duplicate operator: " + id);
        pending.emplace(id, std::move(d));
    }
    std::lock_guard lock(mutex);
    for (auto& [id, d] : pending)
        if (entries.contains(id))
            throw std::runtime_error("Duplicate operator: " + id);
    entries.merge(pending);
}
const Descriptor* Registry::find(const std::string& id) const {
    std::lock_guard lock(mutex);
    auto i = entries.find(id);
    return i == entries.end() ? nullptr : &i->second;
}
} // namespace touch
