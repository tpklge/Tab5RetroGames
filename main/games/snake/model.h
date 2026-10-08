#pragma once
#include "config.h"
#include "types.h"
namespace arcade::snake {
struct Food {
    uint16_t cell = 0;
    uint8_t type = 0, active = 0;
    uint32_t remaining = 0;
};
struct State {
    Options options{};
    Random random{};
    uint16_t body[Capacity]{}, head = 0, length = 5, growing = 0, previous_head = 0,
                               previous_tail = 0;
    uint8_t walls[Capacity]{}, direction = 1, pending[2]{}, pending_count = 0;
    Food food[3]{};
    FoodRule rules[6]{};
    uint32_t elapsed = 0, score = 0, clock = 0, spawn_clock = 0, slow_ms = 0, multiplier_ms = 0,
             flash_ms = 0;
    uint16_t eaten = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    void turn(uint8_t direction);
    bool spawn_food(unsigned slot, uint8_t type);
    uint16_t segment(unsigned index) const { return state.body[(state.head + index) % Capacity]; }
    bool occupied(uint16_t cell) const;
    uint32_t interval() const;
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const;

  private:
    void advance();
    void build_map();
    void finish(Status status);
};
} // namespace arcade::snake
