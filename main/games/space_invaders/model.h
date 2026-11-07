#pragma once
#include "config.h"
#include "types.h"
namespace arcade::invaders {
struct Shot { float x = 0, y = 0; uint8_t active = 0, enemy = 0; };
struct State {
    Options options{};
    Random random{};
    uint8_t aliens[Count]{}, shields[4 * ShieldRows * ShieldColumns]{};
    Shot shots[MaxShots]{};
    float player = Width / 2, formation_x = 90, formation_y = 65, ufo = -40;
    int8_t direction = 1;
    uint8_t lives = 3, ufo_active = 0;
    uint16_t level = 1;
    uint32_t elapsed = 0, score = 0, cooldown = 0, march = 0, enemy_clock = 0,
             invulnerable = 0, wave_delay = 0, ufo_clock = 0, remainder = 0;
    Status status = Status::Running;
};
class Model {
 public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    void wave();
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
 private:
    void tick(const Controls &controls);
    void fire(float x, float y, bool enemy);
    void finish(Status status);
};
}
