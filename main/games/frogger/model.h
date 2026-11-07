#pragma once
#include "config.h"
#include "types.h"
namespace arcade::frogger {
struct Lane { float offset = 0, speed = 0; uint16_t length = 0, period = 0; };
struct State {
    Options options{};
    Random random{};
    Lane lanes[LaneCount]{};
    float x = Width / 2, previous_x = Width / 2;
    uint32_t elapsed = 0, score = 0, remaining_ms = 40000, cooldown = 0, respawn_ms = 0,
             transition = 0, remainder = 0, crossings = 0;
    uint16_t level = 1;
    uint8_t row = StartRow, previous_row = StartRow, best_row = StartRow, homes = 0,
            lives = 3, queued = 4, reason = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    uint32_t crossing_time() const;
    bool supported(unsigned lane, float x) const;
    bool traffic(unsigned lane, float x) const;
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, state.crossings, state.level, state.status == Status::Won}; }
  private:
    void prepare();
    void respawn();
    void die(uint8_t reason);
    void finish(Status status);
    void tick(const Controls &controls);
};
}
