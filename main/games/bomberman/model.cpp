#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::bomberman {
namespace {
void reduce(uint32_t &timer) { timer = timer > 10 ? timer - 10 : 0; }
int requested(const Controls &c, bool held) {
    const Action actions[] = {Action::Up, Action::Right, Action::Down, Action::Left};
    for (int i = 0; i < 4; ++i) if (held ? c.down(actions[i]) : c.hit(actions[i])) return i;
    return 4;
}
bool stone(int cell) {
    int x = cell % Width, y = cell / Width;
    return !x || x == Width - 1 || !y || y == Height - 1 || (!(x % 2) && !(y % 2));
}
}
int Model::neighbor(int cell, int direction) const {
    if (cell < 0 || cell >= Cells || direction < 0 || direction > 3) return -1;
    const int dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
    int x = cell % Width + dx[direction], y = cell / Width + dy[direction];
    return x < 0 || x >= Width || y < 0 || y >= Height ? -1 : y * Width + x;
}
bool Model::free(int cell) const {
    if (cell < 0 || cell >= Cells || state.walls[cell] || state.crates[cell]) return false;
    for (const auto &b : state.bombs) if (b.active && b.cell == cell) return false;
    return true;
}
unsigned Model::enemy_count() const { unsigned n = 0; for (auto e : state.enemies) n += e.active; return n; }
uint32_t Model::enemy_interval(unsigned type) const {
    return uint32_t(std::max(220, 560 - state.options.difficulty * 60 - std::min(100, int(state.level - 1) * 20) - int(type) * 25));
}
void Model::reset_player() {
    state.player = state.previous = Start; state.queued = 4; state.plant_request = 0;
    state.cooldown = 0; state.invulnerable_ms = 1500;
}
void Model::prepare() {
    for (int i = 0; i < Cells; ++i) {
        state.walls[i] = uint8_t(stone(i));
        state.crates[i] = uint8_t(!state.walls[i] && state.random.range(100) < 43 + state.options.difficulty * 6);
        state.items[i] = 0; state.flames[i] = 0;
    }
    for (int cell : {Start, Start+1, Start+2, Start+Width, Start+2*Width, Start+2*Width+1, Start+2+Width}) state.crates[cell] = 0;
    unsigned crates = 0;
    for (int i = 0; i < Cells; ++i) if (state.crates[i]) {
        ++crates;
        if (crates % 9 == 0) state.items[i] = 1;
        else if (crates % 13 == 0) state.items[i] = 2;
        else if (crates % 17 == 0) state.items[i] = 3;
    }
    for (int i = 0; i < 3; ++i) {
        constexpr int cells[] = {3 * Width + 3, Width + 5, 5 * Width + 1};
        state.crates[cells[i]] = 1; state.items[cells[i]] = uint8_t(i + 1);
    }
    state.crates[Exit] = 1; state.items[Exit] = 0;
    for (auto &b : state.bombs) b = Bomb{};
    for (auto &e : state.enemies) e = Enemy{};
    unsigned count = std::min<unsigned>(MaxEnemies, 3 + state.options.difficulty + (state.level - 1) / 2);
    for (unsigned i = 0; i < count; ++i) {
        int candidates[Cells], available = 0;
        for (int cell = 0; cell < Cells; ++cell) if (!state.walls[cell] && cell != Exit &&
            std::abs(cell % Width - 1) + std::abs(cell / Width - 1) >= 7) {
            bool used = false;
            for (unsigned j = 0; j < i; ++j) used |= state.enemies[j].cell == cell;
            if (!used) candidates[available++] = cell;
        }
        int cell = candidates[state.random.range(unsigned(available))];
        state.crates[cell] = state.items[cell] = 0;
        auto &e = state.enemies[i]; e.cell = e.previous = uint16_t(cell); e.active = 1;
        e.type = uint8_t(i % 3); e.clock = 200 + i * 30;
    }
    state.remaining_ms = uint32_t(std::max(60000, 180000 - state.options.difficulty * 20000 - std::min(40000, int(state.level - 1) * 5000)));
    state.transition = state.respawn_ms = 0; reset_player();
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{}; state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed; sound_events = 0; prepare();
}
bool Model::plant() {
    unsigned active = 0;
    for (const auto &b : state.bombs) { active += b.active; if (b.active && b.cell == state.player) return false; }
    if (active >= state.capacity || state.flames[state.player] || state.transition || state.respawn_ms || state.status != Status::Running) return false;
    for (auto &b : state.bombs) if (!b.active) {
        b.cell = state.player; b.fuse = FuseMs; b.range = state.range; b.active = 1; sound_events |= Drop; return true;
    }
    return false;
}
bool Model::ignite(int cell) {
    if (cell < 0 || cell >= Cells || state.walls[cell]) return false;
    bool soft = state.crates[cell];
    state.flames[cell] = FlameMs;
    if (soft) { state.crates[cell] = 0; state.score += 25; sound_events |= Eat; }
    else state.items[cell] = 0;
    for (unsigned i = 0; i < MaxBombs; ++i) if (state.bombs[i].active && state.bombs[i].cell == cell) explode(i);
    return !soft;
}
void Model::explode(unsigned index) {
    if (index >= MaxBombs || !state.bombs[index].active) return;
    int cell = state.bombs[index].cell, range = state.bombs[index].range;
    state.bombs[index].active = 0; state.bombs[index].fuse = 0;
    ignite(cell); sound_events |= Combo;
    for (int direction = 0; direction < 4; ++direction) {
        int next = cell;
        for (int distance = 0; distance < range; ++distance) {
            next = neighbor(next, direction);
            if (!ignite(next)) break;
        }
    }
}
bool Model::danger(int cell) const {
    if (cell < 0 || cell >= Cells || state.flames[cell]) return true;
    for (const auto &b : state.bombs) if (b.active) {
        if (b.cell == cell) return true;
        for (int direction = 0; direction < 4; ++direction) {
            int next = b.cell;
            for (int n = 0; n < b.range; ++n) {
                next = neighbor(next, direction);
                if (next < 0 || state.walls[next]) break;
                if (next == cell) return true;
                if (state.crates[next]) break;
            }
        }
    }
    return false;
}
void Model::move_enemy(unsigned index) {
    auto &e = state.enemies[index];
    int candidates[4], count = 0;
    for (int d = 0; d < 4; ++d) {
        int next = neighbor(e.cell, d);
        if (!free(next) || state.flames[next]) continue;
        bool used = false;
        for (unsigned j = 0; j < MaxEnemies; ++j) if (j != index) used |= state.enemies[j].active && state.enemies[j].cell == next;
        if (!used) candidates[count++] = d;
    }
    if (!count) return;
    int chosen = candidates[state.random.range(unsigned(count))];
    if (e.type) {
        int best = 100000; unsigned ties = 0;
        for (int i = 0; i < count; ++i) {
            int next = neighbor(e.cell, candidates[i]);
            int value = std::abs(next % Width - state.player % Width) + std::abs(next / Width - state.player / Width);
            value += danger(next) ? 1000 : 0;
            if (e.type == 1) value += candidates[i] == e.direction ? -4 : 0;
            if (value < best) { best = value; chosen = candidates[i]; ties = 1; }
            else if (value == best && state.random.range(++ties) == 0) chosen = candidates[i];
        }
    }
    e.previous = e.cell; e.direction = uint8_t(chosen); e.cell = uint16_t(neighbor(e.cell, chosen));
}
void Model::finish(Status status) { state.status = status; state.remainder %= 10; sound_events |= GameOver; }
void Model::damage() {
    sound_events |= Crash;
    if (state.options.mode != 3) --state.lives;
    if (!state.lives) { finish(Status::Lost); return; }
    for (auto &b : state.bombs) b.active = 0;
    std::fill_n(state.flames, Cells, uint16_t(0));
    reset_player(); state.respawn_ms = 600;
    state.remaining_ms = uint32_t(std::max(60000, 180000 - state.options.difficulty * 20000 - std::min(40000, int(state.level - 1) * 5000)));
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    int direction = requested(controls, false);
    if (direction != 4 && !state.respawn_ms && !state.transition) state.queued = uint8_t(direction);
    if (controls.hit(Action::Primary) && !state.respawn_ms && !state.transition) state.plant_request = 1;
    state.remainder += std::min<uint32_t>(delta, 1000);
    while (state.remainder >= 10 && state.status == Status::Running) { state.remainder -= 10; tick(controls); }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 10;
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) { state.level = uint16_t(std::min(1000, int(state.level) + 1)); prepare(); sound_events |= Level; }
        return;
    }
    for (auto &f : state.flames) f = f > 10 ? uint16_t(f - 10) : 0;
    reduce(state.invulnerable_ms); reduce(state.cooldown);
    if (state.respawn_ms) { reduce(state.respawn_ms); return; }
    if (state.remaining_ms <= 10) { damage(); return; }
    state.remaining_ms -= 10;
    if (state.plant_request) { state.plant_request = 0; plant(); }
    if (!state.cooldown) {
        int direction = state.queued != 4 ? state.queued : requested(controls, true); state.queued = 4;
        int next = neighbor(state.player, direction);
        if (free(next)) { state.previous = state.player; state.player = uint16_t(next); state.cooldown = movement_interval(); sound_events |= Move; }
    }
    for (unsigned i = 0; i < MaxBombs; ++i) if (state.bombs[i].active) {
        if (state.bombs[i].fuse <= 10) explode(i); else state.bombs[i].fuse -= 10;
    }
    for (auto &e : state.enemies) if (e.active) {
        if (state.flames[e.cell]) { e.active = 0; state.score += 200; sound_events |= Eat; continue; }
        reduce(e.clock);
        if (!e.clock) { move_enemy(unsigned(&e - state.enemies)); e.clock = enemy_interval(e.type); }
    }
    bool hit = state.flames[state.player] != 0;
    for (auto e : state.enemies) hit |= e.active && e.cell == state.player;
    if (hit && !state.invulnerable_ms) { damage(); return; }
    if (!state.flames[state.player] && state.items[state.player]) {
        auto type = state.items[state.player]; state.items[state.player] = 0; state.score += 100; sound_events |= Special;
        if (type == 1) state.range = uint8_t(std::min(6, state.range + 1));
        if (type == 2) state.capacity = uint8_t(std::min(MaxBombs, state.capacity + 1));
        if (type == 3) { state.boots = 1; state.cooldown = std::min(state.cooldown, movement_interval()); }
    }
    if (!state.crates[Exit] && !enemy_count() && state.player == Exit) {
        state.score += state.level * 1000 + state.remaining_ms / 100;
        if (state.options.mode == 0 && state.level >= CampaignLevels) finish(Status::Won);
        else { state.transition = 1200; sound_events |= Bonus; }
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value || unsigned(s.status) > 3 ||
        !s.level || s.level > 1000 || s.lives > 3 || !s.range || s.range > 6 || !s.capacity || s.capacity > MaxBombs ||
        s.boots > 1 || s.queued > 4 || s.plant_request > 1 || s.cooldown > 140 || s.invulnerable_ms > 1500 ||
        s.respawn_ms > 600 || s.transition > 1200 || s.remainder >= 10 || s.remaining_ms > 180000) return false;
    for (int i = 0; i < Cells; ++i)
        if (s.walls[i] != stone(i) || s.crates[i] > 1 || s.items[i] > 3 || s.flames[i] > FlameMs ||
            (s.walls[i] && (s.crates[i] || s.items[i] || s.flames[i]))) return false;
    auto floor = [&](unsigned cell) { return cell < Cells && !s.walls[cell] && !s.crates[cell]; };
    if (!floor(s.player) || !floor(s.previous)) return false;
    unsigned bombs = 0;
    for (unsigned i = 0; i < MaxBombs; ++i) {
        const auto &b = s.bombs[i];
        if (b.active > 1 || b.cell >= Cells || !b.range || b.range > 6 || b.fuse > FuseMs || (b.active && (!b.fuse || !floor(b.cell)))) return false;
        if (b.active) { ++bombs; for (unsigned j = 0; j < i; ++j) if (s.bombs[j].active && s.bombs[j].cell == b.cell) return false; }
    }
    if (bombs > s.capacity) return false;
    for (auto e : s.enemies)
        if (e.active > 1 || e.type > 2 || e.direction > 3 || e.clock > 1000 || e.cell >= Cells || e.previous >= Cells ||
            (e.active && (!floor(e.cell) || !floor(e.previous)))) return false;
    if ((s.status == Status::Running || s.status == Status::Paused) && !s.lives) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0; return true;
}
}
