#include "games/space_invaders/model.h"
#include <cassert>
#include <algorithm>
#include <limits>
#include <cstdio>
using namespace arcade;
using namespace arcade::invaders;
int main() {
    Model m; m.begin({});
    Controls right; right.held[size_t(Action::Right)] = 1;
    m.step(1000, right); assert(m.state.player > 360);
    m.step(1000, right); assert(m.state.player <= Width - 18);
    m.state.status = Status::Paused;
    auto paused = m.save(); m.step(100, right); assert(paused == m.save());
    m.begin({});
    m.state.shots[0] = {m.state.formation_x, m.state.formation_y + 5, 1, 0};
    m.step(10, {}); assert(!m.state.aliens[0] && m.state.score == 30);
    m.state.shots[0] = {84, 449, 1, 1};
    m.step(10, {}); assert(!m.state.shields[0] && !m.state.shots[0].active);
    m.state.invulnerable = 0;
    m.state.shots[0] = {m.state.player, PlayerY - 3, 1, 1};
    m.step(10, {}); assert(m.state.lives == 2);
    m.state.shots[0] = {m.state.player, PlayerY - 3, 1, 1};
    m.step(10, {}); assert(m.state.lives == 2);
    m.state.ufo_active = 1; m.state.ufo = 100;
    m.state.shots[0] = {101, 30, 1, 0};
    m.step(10, {}); assert(!m.state.ufo_active && m.state.score == 180);
    auto saved = m.save(); Model copy; assert(copy.load(saved));
    for (int i = 0; i < 100; ++i) { m.step(10, right); copy.step(10, right); }
    assert(m.save() == copy.save());
    auto before = copy.save(); auto broken = saved; broken.pop_back();
    assert(!copy.load(broken) && before == copy.save());
    State bad = m.state; bad.player = std::numeric_limits<float>::quiet_NaN();
    assert(!copy.load(snapshot(bad)));
    m.begin({}); std::fill_n(m.state.aliens, Count, uint8_t(0));
    m.step(10, {}); assert(m.state.wave_delay);
    m.step(1000, {}); m.step(200, {}); assert(m.state.level == 2 && m.state.aliens[0]);
    Options campaign; campaign.mode = 1; m.begin(campaign); m.state.level = 5;
    std::fill_n(m.state.aliens, Count, uint8_t(0)); m.step(10, {}); assert(m.state.status == Status::Won);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 119990;
    m.step(10, {}); assert(m.state.status == Status::Won);
    m.begin({}); m.state.formation_y = PlayerY - 170;
    m.step(10, {}); assert(m.state.status == Status::Lost);
    m.begin({}); m.state.lives = 1; m.state.invulnerable = 0;
    m.state.shots[0] = {m.state.player, PlayerY - 3, 1, 1}; m.step(10, {});
    assert(m.state.status == Status::Lost && !m.state.lives);
    Model a,b; a.begin({}); b.begin({}); a.step(1000, right);
    for (int i=0; i<100; ++i) b.step(10,right);
    assert(a.save() == b.save());
    Options automatic; automatic.mode = 3; m.begin(automatic); m.step(10, {});
    assert(m.state.shots[0].active && !m.state.shots[0].enemy);
    m.begin({}); m.state.formation_x = 230; m.state.march = 1000;
    float y = m.state.formation_y; m.step(10, {});
    assert(m.state.direction == -1 && m.state.formation_y == y + 22);
    puts("PASS Invaders: movement, pause, collisions, shields, immunity, UFO, waves, modes, game over, validated deterministic save/load");
}
