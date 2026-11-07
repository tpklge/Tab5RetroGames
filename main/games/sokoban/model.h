#pragma once
#include "config.h"
#include "levels.h"
#include "types.h"
namespace arcade::sokoban {
struct Undo { uint8_t player = 0, box_from = NoCell, box_to = NoCell; };
struct State {
    Options options{};
    uint8_t walls[Cells]{}, goals[Cells]{}, boxes[Cells]{};
    Undo history[MaxUndo]{};
    uint32_t elapsed = 0, score = 0, moves = 0, pushes = 0, total_moves = 0, total_pushes = 0,
             repeat_ms = 0, transition = 0, anim_ms = 0;
    uint16_t level = 1, undo_head = 0, undo_count = 0;
    uint8_t player = 0, previous = 0;
    Status status = Status::Running;
};
class Model {
  public:
    State state{};
    uint32_t sound_events = 0;
    void begin(Options options);
    void step(uint32_t delta, const Controls &controls);
    bool move(unsigned direction);
    bool undo();
    void restart_level();
    bool solved() const;
    bool deadlocked() const;
    bool limited() const;
    unsigned move_limit() const;
    unsigned push_limit() const;
    const Puzzle &puzzle() const { return Puzzles[state.options.difficulty * LevelsPerPack + state.level - 1]; }
    std::vector<uint8_t> save() const { return snapshot(state); }
    bool load(const std::vector<uint8_t> &bytes);
    Result result() const;
  private:
    int neighbor(unsigned cell, unsigned direction) const;
    void remember(Undo entry);
    uint32_t reward() const;
};
}
