#pragma once
#include "config.h"
#include "types.h"
namespace arcade::racer {
struct Traffic {
    float x = 0, y = 0, speed = 0;
    uint8_t active = 0, color = 0, passed = 0;
};
struct State {
    Options options{};
    Random random{};
    Traffic traffic[MaxTraffic]{};
    float x = 180, previous_x = 180, speed = 110, distance = 0, health = 100, turbo = 100,
          scroll = 0;
    uint32_t elapsed = 0, spawn_clock = 0, invulnerable = 0, flash_ms = 0, score = 0, overtakes = 0,
             checkpoint = 1;
    uint16_t level = 1;
    uint8_t boosting = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    bool spawn();
    bool collision(const Traffic &car) const;
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const;

  private:
    void finish(Status status);
};
} // namespace arcade::racer
