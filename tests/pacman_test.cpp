#include "games/pacman/model.h"
#include <cassert>
#include <algorithm>
#include <cstdio>
#include <vector>
using namespace arcade;
using namespace arcade::pacman;
Controls press(Action a) { Controls c; c.press[size_t(a)] = 1; return c; }
void calm(Model &m) { for (auto &g : m.state.ghosts) g.respawn_ms = 5000; }
int main() {
    Model m; m.begin({}); assert(m.connected() && m.state.remaining > 200);
    for (int i = 0; i < Cells; ++i) assert(!m.state.walls[i] || !m.state.dots[i]);
    assert(m.neighbor(Start, 0) == -1);
    m.step(100, press(Action::Up)); assert(m.state.player.cell == Start && m.state.player.next == Start);
    m.begin({}); calm(m); auto dots = m.state.remaining;
    m.step(10, press(Action::Right)); assert(m.state.player.next == Start + 1);
    m.step(10, press(Action::Up)); m.step(130, {});
    assert(m.state.player.cell == Start + 1 && m.state.player.next == Start + 1 - Width);
    assert(m.state.remaining == dots - 1 && m.state.score == 10);
    float x,y; m.step(70, {}); m.position(m.state.player, m.player_interval(), x,y);
    assert(x == 13 && y < 21 && y > 20);
    m.state.status = Status::Paused; auto paused = m.save(); m.step(1000, press(Action::Right)); assert(m.save() == paused);
    m.begin({}); calm(m); m.state.player.cell = m.state.player.next = TunnelRow * Width;
    m.state.player.direction = m.state.wanted = 3;
    m.step(m.player_interval(), {}); assert(m.state.player.cell == TunnelRow * Width + Width - 1);
    m.begin({}); calm(m); m.state.dots[Start + 1] = 2;
    m.step(m.player_interval(), press(Action::Right)); assert(m.state.power_ms && m.state.score == 50);
    m.begin({}); calm(m); m.state.invulnerable_ms = 0; m.state.power_ms = 6000;
    for (auto &g : m.state.ghosts) { g.actor.cell = g.actor.next = Start; g.respawn_ms = 0; }
    m.step(10, {}); assert(m.state.score == 3000 && m.state.combo == 4 && m.state.lives == 3);
    for (auto &g : m.state.ghosts) assert(g.respawn_ms);
    m.begin({}); calm(m); m.state.invulnerable_ms = 0;
    m.state.ghosts[0].actor.cell = m.state.ghosts[0].actor.next = Start; m.state.ghosts[0].respawn_ms = 0;
    m.step(10, {}); assert(m.state.lives == 2 && m.state.invulnerable_ms);
    m.state.invulnerable_ms = 0; m.state.lives = 1;
    m.state.ghosts[0].actor.cell = m.state.ghosts[0].actor.next = Start; m.state.ghosts[0].respawn_ms = 0;
    m.step(10, {}); assert(m.state.status == Status::Lost);
    m.begin({}); calm(m); m.state.fruit_ms = 5000;
    m.state.player.cell = Homes[0] - 1; m.state.player.next = Homes[0];
    m.state.player.clock = m.player_interval() - 10; m.state.player.direction = m.state.wanted = 1;
    m.step(10, {}); assert(!m.state.fruit_ms && m.state.score >= 150);
    m.begin({}); calm(m);
    unsigned cleared = 0;
    for (int i = 0; i < Cells && cleared < m.state.total_dots / 3; ++i) if (m.state.dots[i]) {
        m.state.dots[i] = 0; --m.state.remaining; ++cleared;
    }
    m.step(10, {}); assert(m.state.fruit_ms == 9000 && m.state.fruit_spawned == 1);
    m.state.fruit_ms = 1; m.state.power_ms = 1; m.step(10, {}); assert(!m.state.fruit_ms && !m.state.power_ms);
    m.begin({}); m.state.elapsed = 8000; m.state.player.direction = 1;
    m.state.ghosts[3].actor.cell = Start;
    assert(m.ghost_target(0) == Start && m.ghost_target(1) != Start && m.ghost_target(3) != Start);
    std::vector<std::vector<uint8_t>> maps;
    m.begin({});
    for (int stage = 1; stage <= 3; ++stage) {
        assert(m.connected()); maps.emplace_back(m.state.walls, m.state.walls + Cells);
        calm(m); std::fill_n(m.state.dots, Cells, uint8_t(0));
        m.state.dots[Start + 1] = 1; m.state.remaining = 1;
        m.step(m.player_interval() + 10, press(Action::Right));
        if (stage < 3) { assert(m.state.transition); m.step(1000, {}); m.step(200, {}); assert(m.state.level == stage + 1); }
        else assert(m.state.status == Status::Won);
    }
    assert(maps[0] != maps[1] && maps[1] != maps[2]);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 179990; m.step(10, {}); assert(m.state.status == Status::Won);
    Options training; training.mode = 3; m.begin(training); calm(m); m.state.invulnerable_ms = 0;
    m.state.ghosts[0].actor.cell = m.state.ghosts[0].actor.next = Start; m.state.ghosts[0].respawn_ms = 0;
    m.step(10, {}); assert(m.state.lives == 3);
    Model a,b; a.begin({}); b.begin({}); a.step(1000, press(Action::Right)); b.step(10, press(Action::Right));
    for (int i = 0; i < 99; ++i) b.step(10, {});
    assert(a.save() == b.save()); assert(b.load(a.save()));
    for (int i = 0; i < 100; ++i) { a.step(10, {}); b.step(10, {}); } assert(a.save() == b.save());
    auto before = b.save(); auto broken = before; broken.pop_back(); assert(!b.load(broken) && b.save() == before);
    State bad = a.state; bad.dots[0] = 1; assert(!b.load(snapshot(bad)));
    bad = a.state; bad.player.next = Cells; assert(!b.load(snapshot(bad)));
    bad = a.state; ++bad.remaining; assert(!b.load(snapshot(bad)));
    training.difficulty = 3; m.begin(training);
    for (int i = 0; i < 20000; ++i) {
        Controls c; if (i % 20 == 0) c = press(Action((i / 20) % 4));
        m.step(10, c); if (i % 50 == 0) assert(b.load(m.save()));
    }
    puts("PASS Pac-Man: connected varied maps, pellets/power, buffered turns, interpolation, tunnel, ghost personalities/combo/damage, fruit, modes, progress and deterministic validated saves");
}
