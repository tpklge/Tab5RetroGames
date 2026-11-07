#pragma once
#include "config.h"
#include "types.h"
namespace arcade::impact {
struct Enemy {
    float x = 0, y = 0, origin_y = 0;
    uint32_t age = 0, cooldown = 0;
    uint8_t active = 0, type = 0, health = 0;
};
struct Shot {
    float x = 0, y = 0, vx = 0, vy = 0;
    uint8_t active = 0, enemy = 0;
};
struct Pickup { float x = 0, y = 0; uint8_t active = 0, type = 0; };
struct State {
    Options options{};
    Random random{};
    Enemy enemies[MaxEnemies]{};
    Shot shots[MaxShots]{};
    Pickup pickups[MaxPickups]{};
    float x = 100, y = Height / 2, scroll = 0, boss_x = Width - 110, boss_y = Height / 2;
    uint32_t elapsed = 0, stage_time = 0, spawn_clock = 0, cooldown = 0, invulnerable = 1500,
             boss_clock = 0, boss_age = 0, transition = 0, flash = 0, score = 0, kills = 0,
             remainder = 0;
    uint16_t level = 1, boss_health = 0, boss_max = 0;
    uint8_t lives = 3, weapon = 1, bombs = 3, boss_active = 0;
    Status status = Status::Running;
};
class Model {
 public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    void start_boss();
    bool spawn();
    void drop(float x, float y, uint8_t type);
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
 private:
    void tick(const Controls &controls);
    void fire(float x, float y, float vx, float vy, bool enemy);
    void damage();
    void kill(Enemy &enemy);
    void hit_boss(unsigned damage);
    void finish(Status status);
};
}
