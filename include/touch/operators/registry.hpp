#pragma once
#include <cmath>
#include <map>
#include <mutex>
#include <set>
#include <touch/operators/types.hpp>
namespace touch {
class Registry {
    std::map<std::string, Descriptor> entries;
    mutable std::mutex mutex;

  public:
    void add(Descriptor);
    void add_batch(std::vector<Descriptor>);
    const Descriptor* find(const std::string&) const;
    const auto& all() const {
        return entries;
    }
    void remove(const std::string& id) {
        std::lock_guard lock(mutex);
        entries.erase(id);
    }
};

} // namespace touch
