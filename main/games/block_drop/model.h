#pragma once
#include "config.h"
#include "types.h"
namespace arcade::block {
struct Cell {
    int x, y;
};
struct State {
    uint8_t board[Rows][Columns]{};
    Options options{};
    Random random{};
    int8_t queue[14]{}, bag[7]{}, hold = None, piece = I;
    int8_t x = 3, y = 2, rotation = 0, bag_pos = 7, queue_count = 0;
    uint8_t hold_used = 0, lock_resets = 0, last_rotation = 0, kick = 0, back_to_back = 0;
    int16_t combo = -1;
    uint16_t level = 1, lines = 0;
    uint32_t score = 0, elapsed = 0, gravity = 0, lock = 0, clear_ms = 0, clear_mask = 0,
             feedback_ms = 0;
    uint8_t last_clear = 0, spin = 0;
    uint8_t grid = 1;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta_ms, const Controls &controls);
    bool move(int dx, int dy);
    bool rotate(int direction);
    void hard_drop();
    void reserve();
    bool collision(int x, int y, int rotation, int piece) const;
    std::array<Cell, 4> cells(int piece, int rotation) const;
    int ghost_y() const;
    void lock_piece();
    bool load(const std::vector<uint8_t> &bytes);
    std::vector<uint8_t> save() const { return snapshot(state); }
    Result result() const;

  private:
    void spawn();
    int8_t draw_piece();
    void refill_queue();
    void remove_lines();
    void finish(Status status);
    uint32_t interval() const;
};
} // namespace arcade::block
