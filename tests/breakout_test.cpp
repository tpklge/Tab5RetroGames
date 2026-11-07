#include "games/breakout/model.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
using namespace arcade;
using namespace arcade::breakout;
Controls action(Action a, bool press = false) {
    Controls c; (press ? c.press : c.held)[size_t(a)] = 1; return c;
}
void flying(Model &m, Ball ball) {
    m.state.waiting = 0;
    for (auto &b : m.state.balls) b = {};
    ball.active = 1; m.state.balls[0] = ball;
}
void target(Model &m, int index, uint8_t hp, bool last = false) {
    std::fill_n(m.state.bricks, BrickCount, uint8_t(0));
    m.state.bricks[index] = hp;
    m.state.remaining = 1;
    if (!last) { m.state.bricks[BrickCount - 1] = 1; m.state.remaining = 2; }
}
unsigned balls(const Model &m) { unsigned n = 0; for (auto b : m.state.balls) n += b.active; return n; }
int main() {
    Model m; m.begin({});
    assert(m.state.remaining == 50 && m.state.waiting && m.state.lives == 3);
    m.step(100, action(Action::Right)); assert(m.state.paddle > 400 && m.state.balls[0].x == m.state.paddle);
    for (int i = 0; i < 4; ++i) m.step(1000, action(Action::Right));
    assert(m.state.paddle == Width - m.paddle_width() / 2);
    m.step(1, action(Action::Primary, true)); assert(!m.state.waiting && m.state.balls[0].vy < 0);
    m.state.status = Status::Paused; auto paused = m.save(); m.step(500, action(Action::Left)); assert(m.save() == paused);
    m.begin({}); flying(m, {Radius + .1f, 350, -300, -100, 1}); m.step(5, {});
    assert(m.state.balls[0].x >= Radius && m.state.balls[0].vx > 0);
    flying(m, {400, Radius + .1f, 50, -300, 1}); m.step(5, {});
    assert(m.state.balls[0].y >= Radius && m.state.balls[0].vy > 0);
    for (float offset : {-40.f, 0.f, 40.f}) {
        m.begin({}); flying(m, {400 + offset, PaddleY - PaddleHeight / 2 - Radius - .5f, 0, 310, 1});
        m.step(5, {}); assert(m.state.balls[0].vy < 0);
        assert(offset == 0 || offset * m.state.balls[0].vx > 0);
    }
    m.begin({}); target(m, 0, 2);
    flying(m, {BrickLeft + 35.f, BrickTop + BrickHeight + Radius + .5f, 0, -310, 1});
    m.step(5, {}); assert(m.state.bricks[0] == 1 && m.state.remaining == 2 && m.state.balls[0].vy > 0);
    auto score = m.state.score;
    flying(m, {BrickLeft + 35.f, BrickTop + BrickHeight + Radius + .5f, 0, -310, 1});
    m.step(5, {}); assert(!m.state.bricks[0] && m.state.remaining == 1 && m.state.score > score);
    m.begin({}); target(m, 0, 1); m.state.destroyed = 6;
    flying(m, {BrickLeft - Radius - .5f, BrickTop + 12.f, 300, -100, 1});
    m.step(5, {}); assert(!m.state.bricks[0] && m.state.balls[0].vx < 0 && m.state.pickups[0].active);
    assert(m.state.pickups[0].type == uint8_t(Power::Expand));
    m.begin({}); auto width = m.paddle_width(); m.apply(Power::Expand);
    assert(m.paddle_width() == width + 60 && m.state.expanded_ms == 15000);
    m.state.expanded_ms = 5; m.step(5, {}); assert(m.paddle_width() == width);
    m.drop(m.state.paddle, PaddleY - 1, Power::Expand);
    m.step(5, action(Action::Primary, true)); assert(m.state.expanded_ms && m.state.score == 100);
    m.apply(Power::Multiball); assert(balls(m) == 3);
    m.apply(Power::Multiball); assert(balls(m) == 3);
    float normal = m.ball_speed(); m.apply(Power::Slow); assert(m.ball_speed() < normal);
    m.state.slow_ms = 5; m.step(5, {}); assert(m.ball_speed() == normal);
    m.state.balls[0] = {20, Height + Radius - .1f, 0, 310, 1}; m.step(5, {});
    assert(balls(m) == 2 && m.state.lives == 3 && !m.state.waiting);
    m.begin({}); flying(m, {20, Height + Radius - .1f, 0, 310, 1}); m.step(5, {});
    assert(m.state.waiting && m.state.lives == 2);
    m.state.lives = 1; flying(m, {20, Height + Radius - .1f, 0, 310, 1}); m.step(5, {});
    assert(m.state.status == Status::Lost && !m.state.lives);
    Options training; training.mode = 3; m.begin(training);
    flying(m, {20, Height + Radius - .1f, 0, 310, 1}); m.step(5, {});
    assert(m.state.lives == 3 && m.state.waiting);
    m.begin({}); target(m, 0, 1, true);
    flying(m, {BrickLeft + 35.f, BrickTop + BrickHeight + Radius + .5f, 0, -310, 1}); m.step(5, {});
    assert(m.state.transition == 1200); m.step(1000, {}); m.step(200, {});
    assert(m.state.level == 2 && m.state.waiting && m.state.remaining > 0);
    m.state.level = CampaignLevels; target(m, 0, 1, true);
    flying(m, {BrickLeft + 35.f, BrickTop + BrickHeight + Radius + .5f, 0, -310, 1}); m.step(5, {});
    assert(m.state.status == Status::Won);
    Options timed; timed.mode = 2; m.begin(timed); m.state.elapsed = 179995; m.step(5, {});
    assert(m.state.status == Status::Won);
    Model a, b; a.begin({}); b.begin({}); a.step(0, action(Action::Primary, true)); b.step(0, action(Action::Primary, true));
    a.apply(Power::Multiball); b.apply(Power::Multiball);
    a.step(500, action(Action::Right)); for (int i = 0; i < 100; ++i) b.step(5, action(Action::Right));
    assert(a.save() == b.save()); Model copy; assert(copy.load(a.save()));
    for (int i = 0; i < 100; ++i) { a.step(5, action(Action::Left)); copy.step(5, action(Action::Left)); }
    assert(a.save() == copy.save()); auto before = copy.save(); auto broken = before; broken.pop_back();
    assert(!copy.load(broken) && copy.save() == before);
    State bad = a.state; bad.bricks[0] = 4; assert(!copy.load(snapshot(bad)));
    bad = a.state; ++bad.remaining; assert(!copy.load(snapshot(bad)));
    bad = a.state; bad.balls[0].vx = std::numeric_limits<float>::quiet_NaN(); assert(!copy.load(snapshot(bad)));
    // Max-speed substeps and save validity over a long run with an autopilot.
    training.difficulty = 3; m.begin(training); m.state.level = 1000;
    for (int i = 0; i < 20000; ++i) {
        Controls c;
        if (m.state.waiting) c.press[size_t(Action::Primary)] = 1;
        float x = m.state.balls[0].x;
        c.held[size_t(x < m.state.paddle ? Action::Left : Action::Right)] = std::abs(x - m.state.paddle) > 6;
        m.step(5, c);
        if (i % 50 == 0) assert(copy.load(m.save()));
    }
    puts("PASS Breakout: serve, paddle/angles/walls/bricks, resistant blocks, drops, multiball, effects, lives, phases, modes, max-speed simulation and deterministic validated saves");
}
