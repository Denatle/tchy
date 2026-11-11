#include <touch/engine/runtime.hpp>
namespace touch {
Runtime::Runtime(Registry& r, AudioOutputs& outputs, Clock& timer)
    : registry(r), audio_outputs(outputs), clock(timer), worker([this]() {
          Evaluator evaluator(registry);
          uint64_t current = 0;
          int width = 1280, height = 720, fps = 30;
          auto session_start = clock.now(), stats_start = session_start;
          uint64_t last_frame = UINT64_MAX, dropped = 0;
          int frames = 0;
          float measured_fps = 0;
          std::unique_ptr<AudioOutput> stream;
          auto next = clock.now();
          while (!quit.load()) {
              std::shared_ptr<const Graph> snapshot;
              bool run, mute;
              float volume;
              Id inspect;
              uint64_t revision;
              {
                  std::lock_guard lock(mutex);
                  snapshot = graph;
                  run = active;
                  mute = muted;
                  revision = session;
                  volume = gain;
                  inspect = inspection;
              }
              const Graph& g = *snapshot;
              if (!run) {
                  evaluator.reset();
                  stream.reset();
                  {
                      std::lock_guard lock(mutex);
                      worker_stopped = true;
                      stopped.notify_all();
                  }
                  clock.sleep_until(clock.now() + std::chrono::milliseconds(10));
                  continue;
              }
              if (current != revision) {
                  current = revision;
                  width = g.width;
                  height = g.height;
                  fps = g.fps;
                  evaluator.reset();
                  session_start = clock.now();
                  stats_start = session_start;
                  last_frame = UINT64_MAX;
                  dropped = 0;
                  frames = 0;
                  measured_fps = 0;
                  next = clock.now();
              }
              double seconds = std::chrono::duration<double>(clock.now() - session_start).count();
              uint64_t frame = uint64_t(seconds * fps);
              bool due = frame != last_frame;
              if (last_frame != UINT64_MAX && frame > last_frame + 1)
                  dropped += frame - last_frame - 1;
              last_frame = frame;
              Context c{width, height, frame, seconds, due, inspect};
              Monitor result;
              try {
                  result = evaluator.tick(g, c);
              } catch (const std::exception& e) {
                  result.errors[0] = e.what();
              }
              if (mute)
                  stream.reset();
              if (!mute && result.audio) {
                  try {
                      if (!stream)
                          stream = audio_outputs.open();
                      stream->write(result.audio, volume);
                  } catch (const std::exception& e) {
                      stream.reset();
                      result.errors[g.audio] = e.what();
                  }
              }
              if (due && result.frame)
                  ++frames;
              double stats_seconds =
                  std::chrono::duration<double>(clock.now() - stats_start).count();
              if (stats_seconds >= 1) {
                  measured_fps = frames / stats_seconds;
                  frames = 0;
                  stats_start = clock.now();
              }
              result.fps = measured_fps;
              result.dropped = dropped;
              {
                  std::lock_guard lock(mutex);
                  if (active)
                      monitor = std::move(result);
              }
              next += std::chrono::milliseconds(10);
              auto now = clock.now();
              if (next < now)
                  next = now;
              clock.sleep_until(next);
          }
          stream.reset();
      }) {}
Runtime::~Runtime() {
    quit.store(true);
    if (worker.joinable())
        worker.join();
}
void Runtime::update(Graph g) {
    std::lock_guard lock(mutex);
    graph = std::make_shared<const Graph>(std::move(g));
}
void Runtime::run(bool v) {
    std::unique_lock lock(mutex);
    if (v && !active) {
        ++session;
        worker_stopped = false;
    }
    active = v;
    if (!v) {
        monitor = {};
        stopped.wait(lock, [&] { return worker_stopped; });
    }
}
void Runtime::volume(float db) {
    std::lock_guard lock(mutex);
    gain = std::pow(10, std::clamp(db, -60.f, 12.f) / 20);
}
void Runtime::inspect(Id id) {
    std::lock_guard lock(mutex);
    inspection = id;
}
void Runtime::mute(bool v) {
    std::lock_guard lock(mutex);
    muted = v;
}
bool Runtime::running() const {
    std::lock_guard lock(mutex);
    return active;
}
Monitor Runtime::snapshot() const {
    std::lock_guard lock(mutex);
    return monitor;
}
} // namespace touch
