#include "games/frogger/model.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
using namespace arcade;
using namespace arcade::frogger;
Controls press(Action a) { Controls c; c.press[size_t(a)] = 1; return c; }
void home(Model &m, int index) {
    m.state.row = m.state.previous_row = 1; m.state.x = m.state.previous_x = HomeX[index];
    m.state.cooldown = m.state.respawn_ms = 0; m.state.lanes[0].speed = 0;
    m.step(10, press(Action::Up));
}
int main() {
    Model m; m.begin({}); assert(m.state.row == StartRow && m.state.remaining_ms == m.crossing_time());
    m.step(3, press(Action::Up)); assert(m.state.row == StartRow);
    Model partial; assert(partial.load(m.save())); m.step(7, {}); partial.step(7, {});
    assert(m.state.row == 11 && m.save() == partial.save());
    float x = m.state.x; m.step(10, press(Action::Right)); assert(m.state.x == x);
    m.step(130, {}); assert(m.state.x == x + CellWidth);
    m.state.status = Status::Paused; auto paused = m.save(); m.step(1000, press(Action::Up)); assert(m.save() == paused);
    m.begin({}); m.state.lanes[4].offset = 100; m.state.lanes[4].speed = 0;
    assert(m.traffic(4, 110) && !m.traffic(4, 100 + m.state.lanes[4].length + 30));
    m.state.row = 6; m.state.x = 110; m.step(10, {});
    assert(m.state.lives == 2 && m.state.respawn_ms && m.state.reason == 1);
    m.begin({}); m.state.lanes[0].offset = 100; m.state.lanes[0].speed = 50;
    m.state.row = 1; m.state.x = 150; m.step(100, {});
    assert(std::abs(m.state.x - 155) < .01f && m.state.lives == 3);
    m.state.x = m.state.lanes[0].offset + m.state.lanes[0].length + 20; m.step(10, {});
    assert(m.state.lives == 2 && m.state.reason == 2);
    m.begin({}); m.state.row = 1; m.state.x = 16; m.state.lanes[0].offset = 0; m.state.lanes[0].speed = -200;
    m.step(10, {}); assert(m.state.lives == 2);
    m.begin({}); home(m, 2); assert(m.state.homes == 4 && m.state.crossings == 1 && m.state.score > 200);
    home(m, 2); assert(m.state.lives == 2 && m.state.homes == 4);
    m.begin({});
    for (int i = 0; i < HomeCount; ++i) home(m, i);
    assert(m.state.homes == 31 && m.state.transition);
    m.step(1000, {}); m.step(200, {}); assert(m.state.level == 2 && !m.state.homes && m.state.row == StartRow);
    m.state.level = 3; m.state.homes = 15; home(m, 4); assert(m.state.status == Status::Won);
    m.begin({}); m.state.remaining_ms = 10; m.step(10, {}); assert(m.state.reason == 3 && m.state.lives == 2);
    m.state.respawn_ms = 0; m.state.lives = 1; m.state.remaining_ms = 10; m.step(10, {});
    assert(m.state.status == Status::Lost && !m.state.lives);
    Options training; training.mode = 3; m.begin(training); m.state.remaining_ms = 10; m.step(10, {});
    assert(m.state.lives == 3 && m.state.respawn_ms);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 179990; m.step(10, {}); assert(m.state.status == Status::Won);
    Model a,b; a.begin({}); b.begin({}); a.step(1000, {}); for (int i = 0; i < 100; ++i) b.step(10, {});
    assert(a.save() == b.save()); assert(b.load(a.save()));
    State bad = a.state; bad.lanes[0].period = 0; assert(!b.load(snapshot(bad)));
    bad = a.state; bad.x = std::numeric_limits<float>::quiet_NaN(); assert(!b.load(snapshot(bad)));
    bad = a.state; bad.homes = 255; assert(!b.load(snapshot(bad)));
    auto bytes = a.save(); bytes.pop_back(); assert(!b.load(bytes));
    training.difficulty = 3; m.begin(training); m.state.level = 1000;
    for (int i = 0; i < 20000; ++i) {
        Controls c; if (i % 20 == 0) c = press(Action((i / 20) % 4));
        m.step(10, c); if (i % 50 == 0) assert(b.load(m.save()));
    }
    puts("PASS Frogger: buffered hops, traffic, logs/carry, water/edges, occupied homes, progress, timer, modes and deterministic validated saves");
}
