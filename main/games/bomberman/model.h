#pragma once
#include "config.h"
#include "types.h"
namespace arcade::bomberman {
struct Bomb { uint32_t fuse = 0; uint16_t cell = 0; uint8_t range = 2, active = 0; };
struct Enemy { uint32_t clock = 0; uint16_t cell = 0, previous = 0; uint8_t type = 0, active = 0, direction = 0; };
struct State {
    Options options{};
    Random random{};
    uint8_t walls[Cells]{}, crates[Cells]{}, items[Cells]{};
    uint16_t flames[Cells]{};
    Bomb bombs[MaxBombs]{};
    Enemy enemies[MaxEnemies]{};
    uint32_t elapsed = 0, score = 0, remaining_ms = 160000, cooldown = 0, invulnerable_ms = 1500,
             respawn_ms = 0, transition = 0, remainder = 0;
    uint16_t player = Start, previous = Start, level = 1;
    uint8_t lives = 3, range = 2, capacity = 1, boots = 0, queued = 4, plant_request = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    bool plant();
    void explode(unsigned index);
    int neighbor(int cell, int direction) const;
    bool free(int cell) const;
    bool danger(int cell) const;
    unsigned enemy_count() const;
    uint32_t movement_interval() const { return state.boots ? 100 : 140; }
    uint32_t enemy_interval(unsigned type) const;
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
  private:
    void prepare();
    void reset_player();
    void damage();
    void tick(const Controls &controls);
    bool ignite(int cell);
    void move_enemy(unsigned index);
    void finish(Status status);
};
}
