#include <SDL3/SDL.h>
#include <touch/plugin.h>
#include <touch/plugins/manager.hpp>
namespace touch {
struct PluginManager::Library {
    SDL_SharedObject* object = nullptr;
    std::vector<std::string> ids;
    ~Library() {
        if (object)
            SDL_UnloadObject(object);
    }
};
std::unique_ptr<Operator> plugin_operator(std::shared_ptr<void>, TouchOperatorApi, size_t,
                                          std::vector<Parameter>, size_t, std::vector<Signal>);
PluginManager::~PluginManager() = default;
void PluginManager::load(const std::filesystem::path& path) {
    auto lib = std::make_shared<Library>();
    lib->object = SDL_LoadObject(path.string().c_str());
    if (!lib->object)
        throw std::runtime_error(SDL_GetError());
    auto get = reinterpret_cast<TouchGetApi>(SDL_LoadFunction(lib->object, "touch_plugin_get_api"));
    if (!get)
        throw std::runtime_error("Missing touch_plugin_get_api");
    auto api = get(TOUCH_ABI_VERSION);
    if (!api || api->abi_version != TOUCH_ABI_VERSION ||
        api->struct_size < sizeof(TouchPluginApi) || !api->operators || api->operator_count > 4096)
        throw std::runtime_error("Incompatible plugin ABI");
    std::vector<Descriptor> pending;
    std::set<std::string> ids;
    for (size_t i = 0; i < api->operator_count; ++i) {
        auto a = api->operators[i];
        if (a.struct_size < sizeof(TouchOperatorApi) || !a.create || !a.destroy || !a.process ||
            !a.descriptor_json)
            throw std::runtime_error("Invalid plugin functions");
        auto j = Json::parse(a.descriptor_json);
        Descriptor d;
        d.id = j["id"].str();
        d.label = j["label"].str();
        if (d.id.empty() || registry.find(d.id) || !ids.insert(d.id).second)
            throw std::runtime_error("Duplicate plugin operator");
        for (auto pair : {std::pair{"inputs", &d.inputs}, std::pair{"outputs", &d.outputs}})
            for (size_t k = 0; k < j[pair.first].size(); ++k) {
                auto port = j[pair.first][k];
                int signal = port.has("signal") ? port["signal"].number() : port.number();
                if (port.has("id")) {
                    auto& keys = std::string(pair.first) == "inputs" ? d.input_keys : d.output_keys;
                    keys.push_back(port["id"].str());
                }
                if (signal < 1 || signal > 6)
                    throw std::runtime_error("Invalid signal");
                pair.second->push_back(Signal(signal));
            }
        for (size_t k = 0; k < j["params"].size(); ++k) {
            auto p = j["params"][k];
            d.params.push_back({p["name"].str(), float(p["default"].number()),
                                float(p["min"].number()), float(p["max"].number())});
        }
        if (!d.input_keys.empty())
            for (auto& p : d.params)
                d.input_keys.push_back("param." + p.name);
        for (size_t k = 0; k < j["choices"].size(); ++k) {
            auto v = j["choices"][k];
            Choice c;
            c.name = v["name"].str();
            for (size_t i = 0; i < v["items"].size(); ++i)
                c.items.push_back(v["items"][i].str());
            if (c.items.empty())
                throw std::runtime_error("Empty choice");
            d.choices.push_back(c);
        }
        d.video_rate = j["video_rate"].number() != 0;
        d.cacheable = j["cacheable"].number() != 0;
        d.create = [lib, a, count = d.outputs.size(), params = d.params, inputs = d.inputs.size(),
                    signals = d.outputs] {
            return plugin_operator(lib, a, count, params, inputs, signals);
        };
        lib->ids.push_back(d.id);
        pending.push_back(std::move(d));
    }
    registry.add_batch(std::move(pending));
    libs.push_back(lib);
}
void PluginManager::unload_all(bool running) {
    if (running)
        throw std::runtime_error("Stop the graph before unloading plugins");
    for (auto& lib : libs)
        for (auto& id : lib->ids)
            registry.remove(id);
    libs.clear();
}
} // namespace touch
