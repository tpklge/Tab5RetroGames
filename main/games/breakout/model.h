#pragma once
#include "config.h"
#include "types.h"
namespace arcade::breakout {
struct Ball { float x = 0, y = 0, vx = 0, vy = 0; uint8_t active = 0; };
struct Pickup { float x = 0, y = 0; uint8_t active = 0, type = 0; };
struct State {
    Options options{};
    Random random{};
    uint8_t bricks[BrickCount]{};
    Ball balls[MaxBalls]{};
    Pickup pickups[MaxPickups]{};
    float paddle = Width / 2;
    uint32_t score = 0, elapsed = 0, expanded_ms = 0, slow_ms = 0, transition = 0,
             destroyed = 0, remainder = 0, flash_ms = 0;
    uint16_t level = 1, remaining = 0;
    uint8_t lives = 3, waiting = 1;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    float paddle_width() const;
    float ball_speed() const;
    void apply(Power power);
    void drop(float x, float y, Power power);
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const { return {state.score, state.elapsed, 0, state.level, state.status == Status::Won}; }
  private:
    void prepare_level();
    void serve();
    void launch();
    void tick(const Controls &controls);
    void normalize(Ball &ball) const;
    bool brick_collision(Ball &ball, float previous_x, float previous_y);
    void finish(Status status);
};
}
