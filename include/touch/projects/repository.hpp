#pragma once
#include <filesystem>
#include <touch/graph/graph.hpp>
namespace touch {
Json encode_project(const Graph&, const std::filesystem::path&);
Graph decode_project(const Json&, const std::filesystem::path&, const Registry&);
class ProjectStore {
  public:
    virtual ~ProjectStore() = default;
    virtual Graph load(const std::filesystem::path&, const Registry&) = 0;
    virtual void save(const std::filesystem::path&, const Graph&) = 0;
};
class JsonProjects : public ProjectStore {
  public:
    Graph load(const std::filesystem::path&, const Registry&) override;
    void save(const std::filesystem::path&, const Graph&) override;
};

} // namespace touch
