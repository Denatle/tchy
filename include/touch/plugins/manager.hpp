#pragma once
#include <filesystem>
#include <touch/operators/registry.hpp>
namespace touch {
class PluginManager {
    struct Library;
    Registry& registry;
    std::vector<std::shared_ptr<Library>> libs;

  public:
    explicit PluginManager(Registry& r) : registry(r) {}
    ~PluginManager();
    void load(const std::filesystem::path&);
    void unload_all(bool running);
};

} // namespace touch
