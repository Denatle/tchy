#include <algorithm>
#include <fstream>
#include <sstream>
#include <touch/json.hpp>
#include <touch/settings/types.hpp>
namespace touch {
Preferences JsonPreferences::load() {
    Preferences p;
    std::ifstream file(path);
    if (!file)
        return p;
    try {
        std::stringstream text;
        text << file.rdbuf();
        auto j = Json::parse(text.str());
        if (j.has("grid"))
            p.grid = j["grid"].number() != 0;
        if (j.has("inspector"))
            p.inspector = j["inspector"].number() != 0;
        if (j.has("muted"))
            p.muted = j["muted"].number() != 0;
        p.volume_db = std::clamp(float(j["volume_db"].number()), -60.f, 12.f);
        p.background = std::clamp(int(j["background"].number()), 0, 2);
    } catch (...) {
        return {};
    }
    return p;
}
void JsonPreferences::save(const Preferences& p) {
    if (path.empty())
        return;
    std::filesystem::create_directories(path.parent_path());
    Json j;
    j.set("grid", int(p.grid));
    j.set("inspector", int(p.inspector));
    j.set("muted", int(p.muted));
    j.set("volume_db", double(p.volume_db));
    j.set("background", p.background);
    std::ofstream file(path);
    file.exceptions(std::ios::badbit | std::ios::failbit);
    file << j.dump();
}
} // namespace touch
