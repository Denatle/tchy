#pragma once
#include <filesystem>
namespace touch {
struct Preferences {
    bool grid = true, inspector = true, muted = true;
    float volume_db = 0;
    int background = 0;
};
class PreferencesStore {
  public:
    virtual ~PreferencesStore() = default;
    virtual Preferences load() = 0;
    virtual void save(const Preferences&) = 0;
};
class JsonPreferences : public PreferencesStore {
    std::filesystem::path path;

  public:
    explicit JsonPreferences(std::filesystem::path file) : path(std::move(file)) {}
    Preferences load() override;
    void save(const Preferences&) override;
};
} // namespace touch
