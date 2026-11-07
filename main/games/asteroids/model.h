#pragma once
#include "config.h"
#include "types.h"
namespace arcade::asteroids {
struct Rock {
    float x = 0, y = 0, vx = 0, vy = 0, angle = 0, spin = 0;
    uint8_t active = 0, size = 1, shape = 0;
};
struct Shot {
    float x = 0, y = 0, vx = 0, vy = 0;
    uint32_t life = 0;
    uint8_t active = 0, enemy = 0;
};
struct State {
    Options options{};
    Random random{};
    Rock rocks[MaxRocks]{};
    Shot shots[MaxShots]{};
    float x = Width / 2, y = Height / 2, vx = 0, vy = 0, angle = Pi * 1.5f,
          ufo_x = 0, ufo_y = 0, ufo_base_y = 0, ufo_vx = 0;
    uint32_t elapsed = 0, score = 0, cooldown = 0, invulnerable_ms = 1800, respawn_ms = 0,
             warp_ms = 0, ufo_clock = 0, ufo_age = 0, ufo_shot_clock = 0, transition = 0, remainder = 0;
    uint16_t level = 1;
    uint8_t lives = 3, thrust = 0, shot_request = 0, warp_request = 0, ufo_active = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    void destroy(unsigned index);
    void spawn_ufo();
    void hyperspace();
    unsigned rock_count() const;
    static float distance_squared(float x, float y, float ox, float oy);
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
  private:
    void wave();
    void reset_ship();
    bool fire(float x, float y, float vx, float vy, bool enemy);
    void damage();
    void tick(const Controls &controls);
    void finish(Status status);
};
}
