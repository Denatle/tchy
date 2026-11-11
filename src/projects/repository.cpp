#include <SDL3/SDL.h>
#include <fstream>
#include <sstream>
#include <touch/projects/repository.hpp>
#ifdef _WIN32
#include <windows.h>
#endif
namespace touch {
Graph JsonProjects::load(const std::filesystem::path& path, const Registry& registry) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open project");
    std::stringstream text;
    text << file.rdbuf();
    return decode_project(Json::parse(text.str()), path, registry);
}
void JsonProjects::save(const std::filesystem::path& path, const Graph& graph) {
    auto text = encode_project(graph, path).dump();
    auto temporary = path;
    temporary += "." + std::to_string(SDL_GetTicksNS()) + ".tmp";
    try {
        std::ofstream out(temporary, std::ios::binary);
        out.exceptions(std::ios::badbit | std::ios::failbit);
        out.write(text.data(), text.size());
        out.flush();
        out.close();
#ifdef _WIN32
        if (!MoveFileExW(temporary.c_str(), path.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace project file");
#else
        std::filesystem::rename(temporary, path);
#endif
    } catch (...) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}
} // namespace touch
