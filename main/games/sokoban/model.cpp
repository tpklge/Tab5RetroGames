#include "model.h"
#include <algorithm>
namespace arcade::sokoban {
namespace {
void reduce(uint32_t &timer, uint32_t delta) { timer = timer > delta ? timer - delta : 0; }
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{}; state.options = options_valid(options) ? options : Options{};
    sound_events = 0; restart_level();
}
void Model::restart_level() {
    const auto &level = puzzle();
    for (int i = 0; i < Cells; ++i) {
        char c = level.map[i];
        state.walls[i] = c == '#'; state.goals[i] = c == '.' || c == '*' || c == '+';
        state.boxes[i] = c == '$' || c == '*';
        if (c == '@' || c == '+') state.player = state.previous = uint8_t(i);
    }
    std::fill_n(state.history, MaxUndo, Undo{});
    state.moves = state.pushes = state.undo_head = state.undo_count = 0;
    state.repeat_ms = state.transition = state.anim_ms = 0;
}
int Model::neighbor(unsigned cell, unsigned direction) const {
    if (cell >= Cells || direction > 3) return -1;
    const int dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
    int x = int(cell % Width) + dx[direction], y = int(cell / Width) + dy[direction];
    return x < 0 || x >= Width || y < 0 || y >= Height ? -1 : y * Width + x;
}
unsigned Model::move_limit() const { return puzzle().moves + 12 - state.options.difficulty * 2; }
unsigned Model::push_limit() const { return puzzle().pushes + 3 - state.options.difficulty / 2; }
bool Model::limited() const {
    return !solved() && ((state.options.mode == 1 && state.moves >= move_limit()) ||
                        (state.options.mode == 2 && state.pushes >= push_limit()));
}
bool Model::solved() const {
    bool any = false;
    for (int i = 0; i < Cells; ++i) if (state.boxes[i]) { any = true; if (!state.goals[i]) return false; }
    return any;
}
bool Model::deadlocked() const {
    for (int i = 0; i < Cells; ++i) if (state.boxes[i] && !state.goals[i]) {
        auto blocked = [&](unsigned d) { int n = neighbor(unsigned(i), d); return n < 0 || state.walls[n]; };
        if ((blocked(0) || blocked(2)) && (blocked(1) || blocked(3))) return true;
    }
    return false;
}
void Model::remember(Undo entry) {
    unsigned slot = (state.undo_head + state.undo_count) % MaxUndo;
    state.history[slot] = entry;
    if (state.undo_count < MaxUndo) ++state.undo_count;
    else state.undo_head = uint16_t((state.undo_head + 1) % MaxUndo);
}
bool Model::move(unsigned direction) {
    if (state.status != Status::Running || state.transition) return false;
    int next = neighbor(state.player, direction);
    if (next < 0 || state.walls[next] || (state.options.mode == 1 && state.moves >= move_limit())) return false;
    Undo entry{state.player, NoCell, NoCell};
    if (state.boxes[next]) {
        int beyond = neighbor(unsigned(next), direction);
        if (beyond < 0 || state.walls[beyond] || state.boxes[beyond] ||
            (state.options.mode == 2 && state.pushes >= push_limit())) return false;
        entry.box_from = uint8_t(next); entry.box_to = uint8_t(beyond);
        state.boxes[next] = 0; state.boxes[beyond] = 1; ++state.pushes; sound_events |= Drop;
    } else sound_events |= Move;
    remember(entry); state.previous = state.player; state.player = uint8_t(next);
    ++state.moves; state.anim_ms = 90;
    if (solved()) { state.transition = 1200; sound_events |= Bonus; }
    return true;
}
bool Model::undo() {
    if (!state.undo_count || !state.moves) return false;
    unsigned slot = (state.undo_head + state.undo_count - 1) % MaxUndo;
    const auto entry = state.history[slot];
    if (entry.player >= Cells || state.walls[entry.player] || state.boxes[entry.player]) return false;
    int direction = -1;
    for (unsigned d = 0; d < 4; ++d) if (neighbor(entry.player, d) == state.player) direction = int(d);
    if (direction < 0) return false;
    if (entry.box_from != NoCell) {
        if (entry.box_from != state.player || entry.box_to >= Cells || !state.pushes ||
            state.boxes[entry.box_from] || !state.boxes[entry.box_to] || state.walls[entry.box_to] ||
            neighbor(entry.box_from, unsigned(direction)) != entry.box_to) return false;
        state.boxes[entry.box_from] = 1; state.boxes[entry.box_to] = 0; --state.pushes;
    } else if (entry.box_to != NoCell) return false;
    state.previous = state.player; state.player = entry.player;
    --state.moves; --state.undo_count; state.anim_ms = 90; state.transition = 0; sound_events |= Rotate;
    return true;
}
uint32_t Model::reward() const {
    return uint32_t(std::max<int64_t>(100, 10000 + state.options.difficulty * 250 - int64_t(state.moves) * 15 - int64_t(state.pushes) * 35));
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    delta = std::min<uint32_t>(delta, 1000); state.elapsed += delta;
    reduce(state.repeat_ms, delta); reduce(state.anim_ms, delta);
    if (controls.hit(Action::Primary)) { restart_level(); sound_events |= Level; return; }
    if (controls.hit(Action::CounterRotate)) { undo(); state.repeat_ms = 160; return; }
    if (state.transition) {
        reduce(state.transition, delta);
        if (!state.transition) {
            state.score += reward(); sound_events |= Level;
            if (state.level >= LevelsPerPack && state.options.mode != 3) { state.status = Status::Won; sound_events |= GameOver; return; }
            state.total_moves += state.moves; state.total_pushes += state.pushes;
            state.level = state.level >= LevelsPerPack ? 1 : uint16_t(state.level + 1); restart_level();
        }
        return;
    }
    const Action actions[] = {Action::Up, Action::Right, Action::Down, Action::Left};
    int wanted = -1; bool press = false;
    for (int d = 0; d < 4; ++d) if (controls.hit(actions[d])) { wanted = d; press = true; break; }
    if (wanted < 0 && !state.repeat_ms)
        for (int d = 0; d < 4; ++d) if (controls.down(actions[d])) { wanted = d; break; }
    if (wanted >= 0) { move(unsigned(wanted)); state.repeat_ms = press ? 160 : 95; }
}
Result Model::result() const {
    return {state.score, state.elapsed, state.options.mode == 2 ? state.total_pushes + state.pushes : state.total_moves + state.moves,
            state.level, state.status == Status::Won};
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || unsigned(s.status) > 3 || s.status == Status::Lost ||
        !s.level || s.level > LevelsPerPack || s.player >= Cells || s.previous >= Cells ||
        s.undo_head >= MaxUndo || s.undo_count > MaxUndo || s.undo_count > s.moves || s.pushes > s.moves ||
        s.repeat_ms > 160 || s.anim_ms > 90 || s.transition > 1200) return false;
    const auto &level = Puzzles[s.options.difficulty * LevelsPerPack + s.level - 1];
    unsigned boxes = 0, goals = 0;
    for (int i = 0; i < Cells; ++i) {
        char c = level.map[i]; bool target = c == '.' || c == '*' || c == '+';
        if (s.walls[i] != (c == '#') || s.goals[i] != target || s.boxes[i] > 1 || (s.walls[i] && s.boxes[i])) return false;
        boxes += s.boxes[i]; goals += s.goals[i];
    }
    if (!goals || boxes != goals || s.walls[s.player] || s.walls[s.previous] || s.boxes[s.player]) return false;
    Model candidate; std::memcpy(&candidate.state, &s, sizeof(s));
    if ((s.options.mode == 1 && s.moves > candidate.move_limit()) || (s.options.mode == 2 && s.pushes > candidate.push_limit()) ||
        (s.transition && !candidate.solved()) || (s.status == Status::Won && (!candidate.solved() || (s.level != LevelsPerPack && s.level != 4) || s.options.mode == 3)) ||
        ((s.status == Status::Running || s.status == Status::Paused) && candidate.solved() && !s.transition)) return false;
    for (unsigned i = 0; i < s.undo_count; ++i) if (!candidate.undo()) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0;
    // The published four-stage catalog used this exact state layout. Its maps
    // remain at positions 1..4 of every pack, so finished saves can continue.
    if (state.status == Status::Won && state.level == 4) {
        state.total_moves += state.moves; state.total_pushes += state.pushes;
        state.level = 5; state.status = Status::Running; restart_level();
    }
    return true;
}
}
