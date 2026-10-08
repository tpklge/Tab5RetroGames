#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::block {
std::array<Cell, 4> Model::cells(int piece, int rotation) const {
    static constexpr Cell spawn[7][4] = {
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}}, {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
        {{1, 0}, {0, 1}, {1, 1}, {2, 1}}, {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}}, {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
        {{2, 0}, {0, 1}, {1, 1}, {2, 1}}};
    auto result =
        std::array<Cell, 4>{spawn[piece][0], spawn[piece][1], spawn[piece][2], spawn[piece][3]};
    if (piece == O)
        return result;
    for (auto &cell : result)
        for (int turn = 0; turn < rotation; ++turn) {
            int old_x = cell.x;
            cell.x = (piece == I ? 3 : 2) - cell.y;
            cell.y = old_x;
        }
    return result;
}
bool Model::collision(int x, int y, int rotation, int piece) const {
    for (auto cell : cells(piece, rotation)) {
        int px = x + cell.x, py = y + cell.y;
        if (px < 0 || px >= Columns || py < 0 || py >= Rows || state.board[py][px])
            return true;
    }
    return false;
}
int8_t Model::draw_piece() {
    if (state.bag_pos >= 7) {
        for (int i = 0; i < 7; ++i)
            state.bag[i] = int8_t(i);
        for (int i = 6; i > 0; --i)
            std::swap(state.bag[i], state.bag[state.random.range(i + 1)]);
        state.bag_pos = 0;
    }
    return state.bag[state.bag_pos++];
}
void Model::refill_queue() {
    while (state.queue_count < 14)
        state.queue[state.queue_count++] = draw_piece();
}
void Model::spawn() {
    refill_queue();
    state.piece = state.queue[0];
    for (int i = 1; i < 14; ++i)
        state.queue[i - 1] = state.queue[i];
    --state.queue_count;
    refill_queue();
    state.x = 3;
    state.y = Hidden - 1;
    state.rotation = 0;
    state.lock = 0;
    state.gravity = 0;
    state.lock_resets = 0;
    state.last_rotation = 0;
    state.hold_used = 0;
    if (collision(state.x, state.y, 0, state.piece))
        finish(Status::Lost);
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{};
    state.options = options;
    state.random.value = options.seed ? options.seed : 1;
    state.level = uint16_t(1 + options.difficulty * 3);
    sound_events = 0;
    spawn();
}
void Model::finish(Status status) {
    state.status = status;
    sound_events |= GameOver;
}
uint32_t Model::interval() const {
    return std::max<uint32_t>(25, uint32_t(1000.f * std::pow(.78f, float(state.level - 1))));
}
bool Model::move(int dx, int dy) {
    if (collision(state.x + dx, state.y + dy, state.rotation, state.piece))
        return false;
    bool grounded = collision(state.x, state.y + 1, state.rotation, state.piece);
    state.x += dx;
    state.y += dy;
    state.last_rotation = 0;
    if (grounded && state.lock_resets < 15) {
        state.lock = 0;
        ++state.lock_resets;
    }
    if (dy > 0)
        state.lock = 0;
    if (dx)
        sound_events |= Move;
    return true;
}
bool Model::rotate(int direction) {
    if (state.piece == O)
        return false;
    const int before = state.rotation, after = (before + (direction > 0 ? 1 : 3)) % 4;
    // SRS offsets use positive-up coordinates; convert to board positive-down.
    static constexpr Cell regular[8][5] = {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
                                           {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
                                           {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
                                           {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
                                           {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
                                           {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
                                           {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
                                           {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}};
    static constexpr Cell line[8][5] = {
        {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}}, {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},
        {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}}, {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},
        {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}}, {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},
        {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}, {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}}};
    int index = before == 0 && after == 1   ? 0
                : before == 1 && after == 0 ? 1
                : before == 1 && after == 2 ? 2
                : before == 2 && after == 1 ? 3
                : before == 2 && after == 3 ? 4
                : before == 3 && after == 2 ? 5
                : before == 3 && after == 0 ? 6
                                            : 7;
    const Cell *offsets = state.piece == I ? line[index] : regular[index];
    for (int test = 0; test < 5; ++test) {
        int x = state.x + offsets[test].x, y = state.y - offsets[test].y;
        if (!collision(x, y, after, state.piece)) {
            bool grounded = collision(state.x, state.y + 1, before, state.piece);
            state.x = x;
            state.y = y;
            state.rotation = after;
            state.last_rotation = 1;
            state.kick = test;
            if (grounded && state.lock_resets < 15) {
                state.lock = 0;
                ++state.lock_resets;
            }
            sound_events |= Rotate;
            return true;
        }
    }
    return false;
}
int Model::ghost_y() const {
    int y = state.y;
    while (!collision(state.x, y + 1, state.rotation, state.piece))
        ++y;
    return y;
}
void Model::hard_drop() {
    int landing = ghost_y();
    if (landing > state.y)
        state.last_rotation = 0;
    state.score += uint32_t(landing - state.y) * 2;
    state.y = landing;
    sound_events |= Drop;
    lock_piece();
}
void Model::reserve() {
    if (state.hold_used)
        return;
    int8_t current = state.piece;
    if (state.hold == None) {
        state.hold = current;
        spawn();
    } else {
        state.piece = state.hold;
        state.hold = current;
        state.x = 3;
        state.y = Hidden - 1;
        state.rotation = 0;
        state.lock = state.gravity = 0;
        state.lock_resets = 0;
        state.last_rotation = 0;
        if (collision(state.x, state.y, 0, state.piece))
            finish(Status::Lost);
    }
    state.hold_used = 1;
}
void Model::remove_lines() {
    int destination = Rows - 1;
    for (int row = Rows - 1; row >= 0; --row)
        if (!(state.clear_mask & (1u << row))) {
            for (int col = 0; col < Columns; ++col)
                state.board[destination][col] = state.board[row][col];
            --destination;
        }
    while (destination >= 0) {
        for (auto &cell : state.board[destination])
            cell = 0;
        --destination;
    }
    state.clear_mask = 0;
    state.clear_ms = 0;
    if (state.options.mode == 1 && state.lines >= 40)
        finish(Status::Won);
    else
        spawn();
}
void Model::lock_piece() {
    bool corners[4]{};
    int count = 0;
    if (state.piece == T && state.last_rotation) {
        const Cell points[4] = {{0, 0}, {2, 0}, {2, 2}, {0, 2}};
        for (int i = 0; i < 4; ++i) {
            int x = state.x + points[i].x, y = state.y + points[i].y;
            corners[i] = x < 0 || x >= Columns || y < 0 || y >= Rows || state.board[y][x];
            count += corners[i];
        }
    }
    state.spin = 0;
    if (count >= 3) {
        const int front[4][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
        state.spin = (corners[front[state.rotation][0]] && corners[front[state.rotation][1]]) ||
                             state.kick == 4
                         ? 2
                         : 1;
    }
    bool above = true;
    for (auto cell : cells(state.piece, state.rotation)) {
        int y = state.y + cell.y, x = state.x + cell.x;
        if (y < 0 || y >= Rows || x < 0 || x >= Columns) {
            finish(Status::Lost);
            return;
        }
        state.board[y][x] = state.piece + 1;
        above &= y < Hidden;
    }
    if (above) {
        finish(Status::Lost);
        return;
    }
    uint8_t cleared = 0;
    state.clear_mask = 0;
    for (int row = 0; row < Rows; ++row) {
        bool full = true;
        for (auto cell : state.board[row])
            full &= cell != 0;
        if (full) {
            ++cleared;
            state.clear_mask |= 1u << row;
        }
    }
    state.last_clear = cleared;
    state.feedback_ms = 1000;
    const uint32_t base[5] = {0, 100, 300, 500, 800}, fullspin[4] = {400, 800, 1200, 1600},
                   mini[3] = {100, 200, 400};
    uint32_t points = state.spin == 2   ? fullspin[std::min<int>(cleared, 3)]
                      : state.spin == 1 ? mini[std::min<int>(cleared, 2)]
                                        : base[std::min<int>(cleared, 4)];
    bool difficult = cleared && (cleared == 4 || state.spin);
    if (difficult && state.back_to_back)
        points = points * 3 / 2;
    state.score += points * state.level;
    if (cleared) {
        if (state.combo < 32767)
            ++state.combo;
        state.score += 50u * uint32_t(state.combo) * state.level;
        if (state.combo > 0)
            sound_events |= Combo;
        state.back_to_back = difficult;
        uint16_t old = state.level;
        state.lines = uint16_t(std::min<unsigned>(65535, unsigned(state.lines) + cleared));
        if (state.options.mode != 3)
            state.level = uint16_t(1 + state.options.difficulty * 3 + state.lines / 10);
        if (old != state.level)
            sound_events |= Level;
        sound_events |= Line;
        state.clear_ms = ClearMs;
    } else {
        state.combo = -1;
        spawn();
    }
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running)
        return;
    state.elapsed += delta;
    state.feedback_ms = state.feedback_ms > delta ? state.feedback_ms - delta : 0;
    if (state.options.mode == 2 && state.elapsed >= 120000) {
        finish(Status::Won);
        return;
    }
    if (state.clear_ms) {
        if (delta >= state.clear_ms)
            remove_lines();
        else
            state.clear_ms -= delta;
        return;
    }
    if (controls.hit(Action::Hold)) {
        reserve();
        return;
    }
    if (controls.hit(Action::Left))
        move(-1, 0);
    if (controls.hit(Action::Right))
        move(1, 0);
    if (controls.hit(Action::Up))
        rotate(1);
    if (controls.hit(Action::CounterRotate))
        rotate(-1);
    if (controls.hit(Action::Primary)) {
        hard_drop();
        return;
    }
    uint32_t fall = controls.down(Action::Down) ? std::min<uint32_t>(35, interval()) : interval();
    state.gravity += delta;
    while (state.gravity >= fall) {
        state.gravity -= fall;
        // A translated piece no longer qualifies for a last-action T-spin.
        if (collision(state.x, state.y + 1, state.rotation, state.piece))
            break;
        ++state.y;
        state.lock = 0;
        state.last_rotation = 0;
        if (controls.down(Action::Down))
            ++state.score;
    }
    if (collision(state.x, state.y + 1, state.rotation, state.piece)) {
        state.lock += delta;
        if (state.lock >= LockMs)
            lock_piece();
    } else
        state.lock = 0;
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State restored;
    if (!decode(bytes, restored) || !options_valid(restored.options) || !restored.random.value ||
        restored.piece < 0 || restored.piece > 6 || restored.hold < -1 || restored.hold > 6 ||
        restored.rotation < 0 || restored.rotation > 3 || restored.x < -3 || restored.x >= 10 ||
        restored.y < 0 || restored.y >= Rows || restored.bag_pos < 0 || restored.bag_pos > 7 ||
        restored.queue_count != 14 || !restored.level || restored.level > 10000 ||
        restored.lock_resets > 15 || unsigned(restored.status) > 3 || restored.spin > 2 ||
        restored.last_clear > 4 || restored.hold_used > 1 || restored.last_rotation > 1 ||
        restored.back_to_back > 1 || restored.grid > 1 || restored.kick > 4 ||
        restored.combo < -1 || restored.clear_ms > ClearMs || (restored.clear_mask >> Rows) ||
        bool(restored.clear_mask) != bool(restored.clear_ms))
        return false;
    for (auto &row : restored.board)
        for (auto cell : row)
            if (cell > 7)
                return false;
    for (auto piece : restored.queue)
        if (piece < 0 || piece > 6)
            return false;
    uint8_t seen = 0;
    for (auto piece : restored.bag) {
        if (piece < 0 || piece > 6 || (seen & (1u << piece)))
            return false;
        seen |= 1u << piece;
    }
    if (!restored.clear_ms &&
        (restored.status == Status::Running || restored.status == Status::Paused)) {
        for (auto cell : cells(restored.piece, restored.rotation)) {
            int x = restored.x + cell.x, y = restored.y + cell.y;
            if (x < 0 || x >= Columns || y < 0 || y >= Rows || restored.board[y][x])
                return false;
        }
    }
    std::memcpy(&state, &restored, sizeof(state));
    sound_events = 0;
    return true;
}
Result Model::result() const {
    return {state.score, state.elapsed, 0, state.level, state.status == Status::Won};
}
} // namespace arcade::block
