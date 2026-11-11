#pragma once
#include <atomic>
#include <condition_variable>
#include <thread>
#include <touch/audio/types.hpp>
#include <touch/engine/plan.hpp>
namespace touch {
struct Monitor {
    Value frame, audio;
    std::map<Id, std::vector<Value>> values;
    std::map<Id, std::string> errors;
    uint64_t dropped = 0;
    float fps = 0;
};
class Evaluator {
    Registry& registry;
    std::map<Id, std::unique_ptr<Operator>> instances;
    std::map<Id, std::string> types;
    std::vector<EvaluationStep> plan;
    std::vector<Id> planned_nodes, planned_roots;
    std::vector<Edge> planned_edges;
    bool plan_valid = false;
    struct Cache {
        std::vector<Value> inputs, outputs;
        std::vector<float> params;
        std::vector<int> choices;
        std::string file, names;
        std::vector<std::array<std::string, 2>> rows;
    };
    std::map<Id, Cache> caches;

  public:
    explicit Evaluator(Registry& r) : registry(r) {}
    Monitor tick(const Graph&, const Context&);
    void reset() {
        instances.clear();
        types.clear();
        caches.clear();
        plan_valid = false;
    }
};
class Runtime {
    Registry& registry;
    AudioOutputs& audio_outputs;
    Clock& clock;
    std::condition_variable stopped;
    bool worker_stopped = true;
    float gain = 1;
    std::atomic<bool> quit = false;
    mutable std::mutex mutex;
    std::shared_ptr<const Graph> graph = std::make_shared<Graph>();
    Monitor monitor;
    bool active = false, muted = true;
    uint64_t session = 0;
    Id inspection = 0;
    std::thread worker;

  public:
    Runtime(Registry&, AudioOutputs&, Clock&);
    ~Runtime();
    void update(Graph);
    void run(bool);
    void mute(bool);
    void volume(float db);
    void inspect(Id id);
    bool running() const;
    Monitor snapshot() const;
};

} // namespace touch
