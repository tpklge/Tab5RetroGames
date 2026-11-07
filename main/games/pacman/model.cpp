#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::pacman {
namespace {
void reduce(uint32_t &timer) { timer = timer > 10 ? timer - 10 : 0; }
}
uint32_t Model::player_interval() const {
    constexpr int periods[] = {180, 150, 125, 105};
    return uint32_t(std::max(80, periods[state.options.difficulty] - std::min(30, int(state.level - 1) * 3)));
}
uint32_t Model::ghost_interval() const {
    constexpr int periods[] = {220, 185, 155, 130};
    return uint32_t(std::max(100, periods[state.options.difficulty] - std::min(30, int(state.level - 1) * 3)) + (state.power_ms ? 80 : 0));
}
int Model::neighbor(int cell, int direction) const {
    if (cell < 0 || cell >= Cells || direction < 0 || direction > 3) return -1;
    int x = cell % Width, y = cell / Width;
    const int dx[] = {0, 1, 0, -1}, dy[] = {-1, 0, 1, 0};
    x += dx[direction]; y += dy[direction];
    if (y == TunnelRow) { if (x < 0) x = Width - 1; if (x >= Width) x = 0; }
    if (x < 0 || x >= Width || y < 0 || y >= Height || state.walls[y * Width + x]) return -1;
    return y * Width + x;
}
void Model::distances(int target, uint16_t *distance) const {
    std::fill_n(distance, Cells, uint16_t(65535));
    if (target < 0 || target >= Cells || state.walls[target]) return;
    uint16_t queue[Cells]; int head = 0, tail = 0;
    queue[tail++] = uint16_t(target); distance[target] = 0;
    while (head < tail) {
        int cell = queue[head++];
        for (int d = 0; d < 4; ++d) {
            int n = neighbor(cell, d);
            if (n >= 0 && distance[n] == 65535) { distance[n] = uint16_t(distance[cell] + 1); queue[tail++] = uint16_t(n); }
        }
    }
}
bool Model::connected() const {
    uint16_t distance[Cells]; distances(Start, distance);
    for (int i = 0; i < Cells; ++i) if (!state.walls[i] && distance[i] == 65535) return false;
    return !state.walls[Start];
}
void Model::reset_actors() {
    state.player = Actor{}; state.wanted = 4; state.power_ms = state.combo = 0; state.invulnerable_ms = 1500;
    for (int i = 0; i < GhostCount; ++i) {
        auto &g = state.ghosts[i]; g.actor = Actor{};
        g.actor.cell = g.actor.next = uint16_t(Homes[i]); g.actor.direction = uint8_t(i % 2 ? 1 : 3);
        g.respawn_ms = 800 + i * 1200;
    }
}
void Model::prepare() {
    int layout = (state.level - 1) % 3;
    std::fill_n(state.walls, Cells, uint8_t(0));
    for (int y = 0; y < Height; ++y) for (int x = 0; x < Width; ++x)
        state.walls[y * Width + x] = uint8_t(!x || x == Width - 1 || !y || y == Height - 1 || (!(x % 2) && !(y % 2)));
    if (layout == 0) {
        for (int y : {4, 8, 14, 18}) for (int x = 3; x <= 7; ++x) state.walls[y * Width + x] = state.walls[y * Width + Width - 1 - x] = 1;
    } else if (layout == 1) {
        for (int x : {4, 8, 16, 20}) for (int y = 4; y <= 8; ++y) state.walls[y * Width + x] = state.walls[(Height - 1 - y) * Width + x] = 1;
    } else {
        for (int y : {6, 16}) for (int x = 6; x <= 10; ++x) state.walls[y * Width + x] = state.walls[y * Width + Width - 1 - x] = 1;
        for (int y = 8; y <= 14; ++y) state.walls[y * Width + 6] = state.walls[y * Width + 18] = 1;
    }
    state.walls[TunnelRow * Width] = state.walls[TunnelRow * Width + Width - 1] = 0;
    for (int home : Homes) state.walls[home] = 0;
    state.walls[Start] = 0;
    uint16_t distance[Cells]; distances(Start, distance);
    // Any isolated gallery is connected to the always-open left ring.
    for (int i = 0; i < Cells; ++i) if (!state.walls[i] && distance[i] == 65535)
        for (int x = 1; x <= i % Width; ++x) state.walls[(i / Width) * Width + x] = 0;
    state.remaining = 0;
    for (int i = 0; i < Cells; ++i) {
        state.dots[i] = state.walls[i] || i == Start ? 0 : 1;
        state.remaining += state.dots[i] != 0;
    }
    for (int corner : Corners) state.dots[corner] = 2;
    state.total_dots = state.remaining;
    state.fruit_ms = state.fruit_spawned = state.transition = 0;
    reset_actors();
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{}; state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed; sound_events = 0; prepare();
}
void Model::finish(Status status) { state.status = status; state.remainder %= 10; sound_events |= GameOver; }
void Model::position(const Actor &a, uint32_t interval, float &x, float &y) const {
    float fraction = float(a.clock) / interval;
    int dx = int(a.next % Width) - int(a.cell % Width), dy = int(a.next / Width) - int(a.cell / Width);
    if (dx > 1) dx = -1;
    if (dx < -1) dx = 1;
    x = a.cell % Width + dx * fraction; y = a.cell / Width + dy * fraction;
    if (x < -.5f) x += Width;
    if (x > Width - .5f) x -= Width;
}
void Model::player_next() {
    auto &a = state.player;
    int next = neighbor(a.cell, state.wanted);
    if (next >= 0) a.direction = state.wanted;
    else next = neighbor(a.cell, a.direction);
    a.next = next >= 0 ? uint16_t(next) : a.cell;
    if (a.next == a.cell) a.clock = 0;
}
int Model::ghost_target(unsigned index) const {
    if (index >= GhostCount) return Start;
    if (state.power_ms) return state.player.cell;
    if (state.elapsed % 27000 < 7000) return Corners[index];
    int target = state.player.cell;
    if (index == 1) for (int i = 0; i < 4; ++i) {
        int n = neighbor(target, state.player.direction); if (n < 0) break; target = n;
    }
    if (index == 2) target = Corners[(state.elapsed / 5000 + index) % GhostCount];
    if (index == 3) {
        int home = state.ghosts[index].actor.cell;
        if (std::abs(home % Width - target % Width) + std::abs(home / Width - target / Width) <= 6) target = Corners[index];
    }
    return target;
}
void Model::ghost_next(unsigned index) {
    auto &a = state.ghosts[index].actor;
    uint16_t distance[Cells]; distances(ghost_target(index), distance);
    int reverse = (a.direction + 2) % 4, alternatives = 0;
    for (int d = 0; d < 4; ++d) alternatives += d != reverse && neighbor(a.cell, d) >= 0;
    int best = -1, best_value = state.power_ms ? -1 : 65536; unsigned ties = 0;
    for (int d = 0; d < 4; ++d) {
        int next = neighbor(a.cell, d);
        if (next < 0 || (alternatives && d == reverse)) continue;
        int value = distance[next];
        bool better = state.power_ms ? value > best_value : value < best_value;
        if (better) { best = d; best_value = value; ties = 1; }
        else if (value == best_value && state.random.range(++ties) == 0) best = d;
    }
    if (best < 0) { a.next = a.cell; a.clock = 0; return; }
    a.direction = uint8_t(best); a.next = uint16_t(neighbor(a.cell, best));
}
void Model::consume() {
    int cell = state.player.cell;
    if (state.dots[cell]) {
        if (state.dots[cell] == 2) {
            uint32_t old_interval = ghost_interval();
            state.power_ms = 7000 - state.options.difficulty * 1000; state.combo = 0;
            for (auto &g : state.ghosts) g.actor.clock = g.actor.clock * ghost_interval() / old_interval;
            state.score += 50; sound_events |= Special;
        } else { state.score += 10; sound_events |= Eat; }
        state.dots[cell] = 0; --state.remaining;
    }
    if (state.fruit_ms && cell == Homes[0]) {
        state.fruit_ms = 0; state.score += 100 + state.level * 50; sound_events |= Bonus;
    }
    if (!state.remaining) {
        state.score += state.level * 1000;
        if (state.options.mode == 0 && state.level >= 3) finish(Status::Won);
        else { state.transition = 1200; sound_events |= Level; }
    }
}
bool Model::collisions() {
    if (state.invulnerable_ms && !state.power_ms) return false;
    float px, py; position(state.player, player_interval(), px, py);
    for (auto &g : state.ghosts) if (!g.respawn_ms) {
        float gx, gy; position(g.actor, ghost_interval(), gx, gy);
        float dx = std::abs(px - gx); dx = std::min(dx, Width - dx);
        float dy = py - gy;
        if (dx * dx + dy * dy >= .30f) continue;
        if (state.power_ms) {
            state.score += 200u << std::min(3, int(state.combo)); state.combo = uint8_t(std::min(4, state.combo + 1));
            int index = int(&g - state.ghosts);
            g.actor.cell = g.actor.next = uint16_t(Homes[index]); g.actor.clock = 0; g.respawn_ms = 2000;
            sound_events |= Combo;
        } else {
            if (state.options.mode != 3) --state.lives;
            sound_events |= Crash;
            if (!state.lives) finish(Status::Lost); else reset_actors();
            return true;
        }
    }
    return false;
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    const Action actions[] = {Action::Up, Action::Right, Action::Down, Action::Left};
    for (int d = 0; d < 4; ++d) if (controls.hit(actions[d])) { state.wanted = uint8_t(d); break; }
    state.remainder += std::min<uint32_t>(delta, 1000);
    while (state.remainder >= 10 && state.status == Status::Running) { state.remainder -= 10; tick(); }
}
void Model::tick() {
    state.elapsed += 10;
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) { state.level = uint16_t(std::min(1000, int(state.level) + 1)); prepare(); }
        return;
    }
    uint32_t previous_interval = ghost_interval();
    reduce(state.power_ms); reduce(state.invulnerable_ms); reduce(state.fruit_ms);
    if (previous_interval != ghost_interval())
        for (auto &g : state.ghosts) g.actor.clock = g.actor.clock * ghost_interval() / previous_interval;
    auto &a = state.player;
    if (a.cell == a.next) player_next();
    if (a.cell != a.next) {
        a.clock += 10;
        if (a.clock >= player_interval()) {
            a.clock -= player_interval(); a.cell = a.next; consume();
            if (state.status != Status::Running || state.transition) return;
            player_next();
        }
    }
    if (collisions()) return;
    for (unsigned i = 0; i < GhostCount; ++i) {
        auto &g = state.ghosts[i];
        if (g.respawn_ms) { reduce(g.respawn_ms); continue; }
        if (g.actor.cell == g.actor.next) ghost_next(i);
        g.actor.clock += 10;
        if (g.actor.clock >= ghost_interval()) {
            g.actor.clock -= ghost_interval(); g.actor.cell = g.actor.next; ghost_next(i);
        }
    }
    collisions();
    if (state.status != Status::Running) return;
    unsigned eaten = state.total_dots - state.remaining;
    if (!state.fruit_ms && state.fruit_spawned < 2 && eaten >= state.total_dots * (state.fruit_spawned + 1) / 3) {
        state.fruit_ms = 9000; ++state.fruit_spawned;
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value || unsigned(s.status) > 3 ||
        !s.level || s.level > 1000 || s.lives > 3 || s.wanted > 4 || s.combo > 4 || s.fruit_spawned > 2 ||
        s.power_ms > 7000 || s.invulnerable_ms > 1500 || s.fruit_ms > 9000 || s.transition > 1200 ||
        s.remainder >= 10 || s.total_dots > Cells || s.remaining > s.total_dots) return false;
    unsigned dots = 0;
    for (int i = 0; i < Cells; ++i) {
        if (s.walls[i] > 1 || s.dots[i] > 2 || (s.walls[i] && s.dots[i])) return false;
        dots += s.dots[i] != 0;
    }
    if (dots != s.remaining || (s.transition && dots) || s.walls[Start]) return false;
    Model candidate; candidate.state = s;
    auto actor_valid = [&](const Actor &a, uint32_t interval) {
        if (a.cell >= Cells || a.next >= Cells || s.walls[a.cell] || s.walls[a.next] || a.direction > 4 || a.clock >= interval) return false;
        return a.cell == a.next || candidate.neighbor(a.cell, a.direction) == a.next;
    };
    if (!actor_valid(s.player, candidate.player_interval()) || !candidate.connected()) return false;
    for (const auto &g : s.ghosts)
        if (g.respawn_ms > 5000 || !actor_valid(g.actor, candidate.ghost_interval())) return false;
    if ((s.status == Status::Running || s.status == Status::Paused) && (!s.lives || (!s.remaining && !s.transition))) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0; return true;
}
}
