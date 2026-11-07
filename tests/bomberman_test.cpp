#include "games/bomberman/model.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
using namespace arcade;
using namespace arcade::bomberman;
Controls press(Action a) { Controls c; c.press[size_t(a)] = 1; return c; }
void empty(Model &m, Options options = {}) {
    m.begin(options);
    std::fill_n(m.state.crates, Cells, uint8_t(0));
    std::fill_n(m.state.items, Cells, uint8_t(0));
    for (auto &e : m.state.enemies) e.active = 0;
}
int main() {
    Model m; m.begin({});
    assert(m.state.crates[Exit] && m.enemy_count() >= 3 && m.free(Start+1) && m.free(Start+Width));
    assert(m.plant() && !m.plant() && !m.free(Start));
    m.step(10, press(Action::Right)); assert(m.state.player == Start+1);
    m.state.cooldown = 0; m.step(10, press(Action::Left)); assert(m.state.player == Start+1);
    m.state.status = Status::Paused; auto paused = m.save(); m.step(1000, press(Action::Primary)); assert(m.save() == paused);
    empty(m); m.state.range = 6; assert(m.plant());
    m.state.crates[Start+2] = 1; m.state.items[Start+2] = 1;
    m.explode(0); assert(m.state.flames[Start] && m.state.flames[Start+1] && m.state.flames[Start+2]);
    assert(!m.state.crates[Start+2] && m.state.items[Start+2] == 1 && !m.state.flames[Start+3]);
    assert(!m.state.flames[2*Width+2]); // Permanent pillar blocks the ray.
    empty(m); m.state.capacity = 3; m.state.range = 3; assert(m.plant());
    m.state.player = Start+2; assert(m.plant()); m.state.player = Start+4; assert(m.plant());
    m.explode(0);
    for (const auto &b : m.state.bombs) assert(!b.active);
    assert(m.state.flames[Start+6]);
    empty(m); m.state.bombs[0] = {10, uint16_t(Start+4), 2, 1};
    m.state.enemies[0].active = 1; m.state.enemies[0].cell = m.state.enemies[0].previous = Start+4;
    m.step(10, {}); assert(!m.state.enemies[0].active && m.state.score == 200);
    empty(m); m.state.invulnerable_ms = 0; assert(m.plant());
    m.step(1000, {}); m.step(1000, {}); m.step(200, {});
    assert(m.state.lives == 2 && m.state.respawn_ms && m.state.player == Start);
    m.state.respawn_ms = m.state.invulnerable_ms = 0; m.state.lives = 1;
    m.state.flames[Start] = 500; m.step(10, {}); assert(m.state.status == Status::Lost);
    empty(m); m.state.items[Start] = 1; m.step(10, {}); assert(m.state.range == 3);
    m.state.items[Start] = 2; m.step(10, {}); assert(m.state.capacity == 2);
    m.state.items[Start] = 3; m.step(10, {}); assert(m.state.boots && m.movement_interval() == 100);
    m.state.range = 6; m.state.capacity = MaxBombs;
    m.state.items[Start] = 1; m.step(10, {}); assert(m.state.range == 6);
    m.state.items[Start] = 2; m.step(10, {}); assert(m.state.capacity == MaxBombs);
    empty(m); assert(m.plant()); assert(m.danger(Start+1) && !m.danger(Start+3));
    m.state.crates[Start+1] = 1; assert(!m.danger(Start+2));
    empty(m); m.state.player = Exit; m.step(10, {}); assert(m.state.transition);
    m.step(1000, {}); m.step(200, {}); assert(m.state.level == 2 && m.state.crates[Exit] && m.enemy_count());
    empty(m); m.state.level = CampaignLevels; m.state.player = Exit; m.step(10, {}); assert(m.state.status == Status::Won);
    empty(m); m.state.remaining_ms = 10; m.step(10, {}); assert(m.state.lives == 2);
    Options train; train.mode = 3; empty(m,train); m.state.remaining_ms = 10; m.step(10, {}); assert(m.state.lives == 3);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 179990; m.step(10, {}); assert(m.state.status == Status::Won);
    Model a,b; a.begin({}); b.begin({}); assert(a.plant() && b.plant());
    a.step(1000, {}); for (int i = 0; i < 100; ++i) b.step(10, {});
    assert(a.save() == b.save()); assert(b.load(a.save()));
    for (int i = 0; i < 150; ++i) { a.step(10, {}); b.step(10, {}); } assert(a.save() == b.save());
    auto before = b.save(); auto bytes = before; bytes.pop_back(); assert(!b.load(bytes) && b.save() == before);
    State bad = a.state; bad.flames[0] = 500; assert(!b.load(snapshot(bad)));
    bad = a.state; bad.player = Cells; assert(!b.load(snapshot(bad)));
    bad = a.state; bad.bombs[0].range = 99; assert(!b.load(snapshot(bad)));
    train.difficulty = 3; m.begin(train);
    for (int i = 0; i < 20000; ++i) {
        Controls c;
        if (i % 16 == 0) c = press(Action((i / 16) % 4));
        if (i % 100 == 0) c.press[size_t(Action::Primary)] = 1;
        m.step(10, c); if (i % 50 == 0) assert(b.load(m.save()));
    }
    puts("PASS Bomberman: movement/own bomb, fuse, walls/crates, chain reactions, upgrades, damage, exit/progression, modes and validated deterministic saves");
}
