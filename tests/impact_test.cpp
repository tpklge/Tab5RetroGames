#include "games/space_impact/model.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
using namespace arcade;
using namespace arcade::impact;
Controls held(Action action) { Controls c; c.held[size_t(action)] = 1; return c; }
void boss_hit(Model &m) {
    m.state.shots[0] = {m.state.boss_x - 10, m.state.boss_y, 0, 0, 1, 0};
    m.step(10, {});
}
int main() {
    Model m; m.begin({});
    Controls diagonal = held(Action::Right); diagonal.held[size_t(Action::Up)] = 1;
    m.step(100, diagonal);
    assert(m.state.x > 100 && m.state.y < Height / 2);
    float diagonal_distance = std::hypot(m.state.x - 100, m.state.y - Height / 2);
    Model straight; straight.begin({}); straight.step(100, held(Action::Right));
    assert(std::abs(diagonal_distance - (straight.state.x - 100)) < .2f);
    for (int i=0;i<4;++i) m.step(1000, diagonal);
    assert(m.state.x <= Width-24 && m.state.y == 24);
    m.state.status = Status::Paused; auto pause = m.save(); m.step(100, diagonal); assert(m.save() == pause);
    m.begin({}); m.step(10, held(Action::Primary));
    assert(m.state.shots[0].active && m.state.shots[0].vx > 0);
    m.begin({}); m.state.enemies[0] = {400, 200, 200, 0, 700, 1, 2, 1};
    m.state.shots[0] = {395, 200, 670, 0, 1, 0}; m.step(10, {});
    assert(!m.state.enemies[0].active && m.state.score == 120 && m.state.kills == 1);
    m.state.kills = 4; m.state.enemies[0] = {400, 200, 200, 0, 700, 1, 2, 1};
    m.state.shots[0] = {395, 200, 670, 0, 1, 0}; m.step(10, {});
    assert(m.state.pickups[0].active && m.state.pickups[0].type == 0);
    m.begin({}); m.drop(m.state.x, m.state.y, 0); m.step(10, {}); assert(m.state.weapon == 2);
    m.state.cooldown = 0; m.step(10, held(Action::Primary));
    unsigned shots = 0; for (auto s : m.state.shots) shots += s.active; assert(shots == 2);
    m.drop(m.state.x, m.state.y, 0); m.step(10, {}); assert(m.state.weapon == 3);
    m.state.lives = 2; m.drop(m.state.x, m.state.y, 1); m.step(10, {}); assert(m.state.lives == 3);
    m.drop(m.state.x, m.state.y, 2); m.step(10, {}); assert(m.state.bombs == 4);
    m.state.invulnerable = 0;
    m.state.shots[0] = {m.state.x, m.state.y, 0, 0, 1, 1}; m.step(10, {});
    assert(m.state.lives == 2 && m.state.weapon == 2);
    m.state.shots[0] = {m.state.x, m.state.y, 0, 0, 1, 1}; m.step(10, {}); assert(m.state.lives == 2);
    m.state.invulnerable = 0; m.state.lives = 1;
    m.state.enemies[0] = {m.state.x, m.state.y, m.state.y, 0, 700, 1, 1, 1}; m.step(10, {});
    assert(m.state.status == Status::Lost && m.state.lives == 0);
    m.begin({}); m.state.stage_time = StageDuration - 10; m.step(10, {});
    assert(m.state.boss_active && m.state.boss_health == m.state.boss_max);
    auto hp = m.state.boss_health; boss_hit(m); assert(m.state.boss_health == hp - 1);
    Controls bomb; bomb.press[size_t(Action::CounterRotate)] = 1;
    m.state.shots[0] = {500, 10, -100, 0, 1, 1};
    m.step(100, bomb); assert(m.state.bombs == 2 && m.state.boss_health == hp - 19);
    assert(!m.state.shots[0].active);
    m.state.boss_health = 1; boss_hit(m); assert(!m.state.boss_active && m.state.transition);
    m.step(1000, {}); m.step(800, {}); assert(m.state.level == 2 && !m.state.transition && !m.state.stage_time);
    m.state.level = 3; m.start_boss(); m.state.boss_health = 1; boss_hit(m); assert(m.state.status == Status::Won);
    Options endless; endless.mode = 1; m.begin(endless); m.state.level = 3; m.start_boss();
    m.state.boss_health = 1; boss_hit(m); assert(m.state.status == Status::Running && m.state.transition);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 179990; m.step(10, {});
    assert(m.state.status == Status::Won);
    Options automatic; automatic.mode = 3; m.begin(automatic); m.step(10, {}); assert(m.state.shots[0].active);
    for (unsigned level = 1; level <= 3; ++level) {
        m.begin({}); m.state.level = level; m.start_boss(); m.step(1000, {});
        unsigned count = 0; for (auto s : m.state.shots) count += s.active && s.enemy;
        assert(count >= 4 + (level-1)*2);
        Model copy; assert(copy.load(m.save()));
        for (int i = 0; i < 50; ++i) { m.step(10, diagonal); copy.step(10, diagonal); }
        assert(m.save() == copy.save());
    }
    Model a,b; a.begin({}); b.begin({}); a.step(1000, diagonal);
    for (int i=0;i<100;++i) b.step(10, diagonal);
    assert(a.save() == b.save());
    auto before = b.save(); auto bad = a.save(); bad.pop_back(); assert(!b.load(bad) && b.save() == before);
    State broken = a.state; broken.boss_health = 600; broken.boss_max = 1; assert(!b.load(snapshot(broken)));
    broken = a.state; broken.x = std::numeric_limits<float>::infinity(); assert(!b.load(snapshot(broken)));
    m.begin({}); for (int i=0;i<MaxEnemies;++i) assert(m.spawn()); assert(!m.spawn());
    puts("PASS Impact: four-axis movement, weapons, pickups, bombs, immunity, collisions, three bosses, progression, modes, validated deterministic save/load");
}
