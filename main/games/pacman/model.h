#pragma once
#include "config.h"
#include "types.h"
namespace arcade::pacman {
struct Actor {
    uint16_t cell = Start, next = Start;
    uint32_t clock = 0;
    uint8_t direction = 4;
};
struct Ghost { Actor actor{}; uint32_t respawn_ms = 0; };
struct State {
    Options options{};
    Random random{};
    uint8_t walls[Cells]{}, dots[Cells]{};
    Actor player{};
    Ghost ghosts[GhostCount]{};
    uint32_t elapsed = 0, score = 0, power_ms = 0, invulnerable_ms = 1500,
             fruit_ms = 0, transition = 0, remainder = 0;
    uint16_t level = 1, remaining = 0, total_dots = 0;
    uint8_t lives = 3, wanted = 4, combo = 0, fruit_spawned = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    int neighbor(int cell, int direction) const;
    int ghost_target(unsigned ghost) const;
    uint32_t player_interval() const;
    uint32_t ghost_interval() const;
    void position(const Actor &actor, uint32_t interval, float &x, float &y) const;
    bool connected() const;
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
  private:
    void prepare();
    void reset_actors();
    void distances(int target, uint16_t *distance) const;
    void player_next();
    void ghost_next(unsigned ghost);
    void consume();
    bool collisions();
    void tick();
    void finish(Status status);
};
}
