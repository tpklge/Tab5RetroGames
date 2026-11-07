#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::breakout {
namespace {
void reduce(uint32_t &timer) { timer = timer > 5 ? timer - 5 : 0; }
bool circle_rect(float x, float y, float left, float top, float w, float h) {
    float dx = x - std::clamp(x, left, left + w), dy = y - std::clamp(y, top, top + h);
    return dx * dx + dy * dy <= Radius * Radius;
}
}
float Model::paddle_width() const {
    return state.expanded_ms ? PaddleWidths[state.options.difficulty] + 60 : PaddleWidths[state.options.difficulty];
}
float Model::ball_speed() const {
    return std::min(630.f, InitialSpeed[state.options.difficulty] + (state.level - 1) * 18.f) * (state.slow_ms ? .72f : 1.f);
}
void Model::normalize(Ball &b) const {
    float speed = ball_speed();
    float length = std::sqrt(b.vx * b.vx + b.vy * b.vy);
    if (length < 1) { b.vx = speed * .35f; b.vy = -speed * .937f; length = std::hypot(b.vx, b.vy); }
    b.vx *= speed / length; b.vy *= speed / length;
    // Prevent near-horizontal trajectories that never return to the platform.
    if (std::abs(b.vy) < speed * .32f) {
        b.vy = std::copysign(speed * .32f, b.vy);
        b.vx = std::copysign(std::sqrt(speed * speed - b.vy * b.vy), b.vx);
    }
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{};
    state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed;
    sound_events = 0;
    prepare_level();
}
void Model::serve() {
    for (auto &b : state.balls) b = Ball{};
    for (auto &p : state.pickups) p = Pickup{};
    state.waiting = 1;
    state.balls[0] = {state.paddle, PaddleY - PaddleHeight / 2 - Radius - 1, 0, 0, 1};
}
void Model::prepare_level() {
    state.remaining = 0;
    state.expanded_ms = state.slow_ms = state.transition = 0;
    state.paddle = Width / 2;
    unsigned layout = (state.level - 1) % CampaignLevels;
    for (int r = 0; r < Rows; ++r) for (int c = 0; c < Columns; ++c) {
        bool occupied = layout == 0 ? r < 5
                      : layout == 1 ? (r + c) % 2 == 0
                      : layout == 2 ? r % 3 == 0 || c == 0 || c == Columns - 1
                      : layout == 3 ? (c + r / 2) % 3 != 1
                      : layout == 4 ? std::abs(c * 2 - 9) + std::abs(r * 2 - 6) <= 10
                                    : true;
        uint8_t hp = occupied ? uint8_t(1 + (r < 2 ? 1 : 0) + (layout == 5 && c >= 3 && c <= 6 ? 1 : 0)) : 0;
        state.bricks[r * Columns + c] = hp;
        state.remaining += occupied;
    }
    serve();
}
void Model::launch() {
    state.waiting = 0;
    auto &b = state.balls[0];
    b.vx = (state.random.range(2) ? 1.f : -1.f) * ball_speed() * .35f;
    b.vy = -ball_speed() * .937f;
    normalize(b);
    sound_events |= Drop;
}
void Model::drop(float x, float y, Power power) {
    for (auto &p : state.pickups) if (!p.active) {
        p = {x, y, 1, uint8_t(power)}; return;
    }
}
void Model::apply(Power power) {
    sound_events |= Special;
    if (power == Power::Expand) {
        state.expanded_ms = 15000;
        state.paddle = std::clamp(state.paddle, paddle_width() / 2, Width - paddle_width() / 2);
    } else if (power == Power::Slow) {
        state.slow_ms = 10000;
        for (auto &b : state.balls) if (b.active && !state.waiting) normalize(b);
    } else if (!state.waiting) {
        Ball source{};
        for (const auto &b : state.balls) if (b.active) { source = b; break; }
        if (!source.active) return;
        int count = 0;
        for (auto &b : state.balls) if (!b.active) {
            b = source;
            float sign = count++ % 2 ? -1.f : 1.f;
            b.vx = sign * ball_speed() * .68f;
            b.vy = -ball_speed() * .73f;
            normalize(b);
        }
    }
}
void Model::finish(Status status) {
    state.status = status; state.remainder %= 5; sound_events |= GameOver;
}
bool Model::brick_collision(Ball &b, float old_x, float old_y) {
    for (int i = 0; i < BrickCount; ++i) if (state.bricks[i]) {
        float left = BrickLeft + (i % Columns) * BrickPitchX, top = BrickTop + (i / Columns) * BrickPitchY;
        if (!circle_rect(b.x, b.y, left, top, BrickWidth, BrickHeight)) continue;
        bool vertical = old_y + Radius <= top || old_y - Radius >= top + BrickHeight;
        bool horizontal = old_x + Radius <= left || old_x - Radius >= left + BrickWidth;
        if (!vertical && !horizontal) {
            float px = std::min(b.x + Radius - left, left + BrickWidth - b.x + Radius);
            float py = std::min(b.y + Radius - top, top + BrickHeight - b.y + Radius);
            vertical = py <= px; horizontal = !vertical;
        }
        if (vertical) {
            b.y = b.vy > 0 ? top - Radius - .05f : top + BrickHeight + Radius + .05f;
            b.vy = -b.vy;
        }
        if (horizontal) {
            b.x = b.vx > 0 ? left - Radius - .05f : left + BrickWidth + Radius + .05f;
            b.vx = -b.vx;
        }
        --state.bricks[i]; state.score += 10; sound_events |= Eat;
        if (!state.bricks[i]) {
            --state.remaining; ++state.destroyed; state.score += 50 + (Rows - i / Columns) * 5;
            if (state.destroyed % 7 == 0)
                drop(left + BrickWidth / 2, top + BrickHeight / 2, Power((state.destroyed / 7 - 1) % 3));
        }
        if (!state.remaining) {
            state.score += state.level * 500;
            if (state.options.mode == 0 && state.level >= CampaignLevels) finish(Status::Won);
            else { state.transition = 1200; sound_events |= Level; }
        }
        return true;
    }
    return false;
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    // An edge launch is consumed here so a <5 ms call cannot discard it.
    if (state.waiting && controls.hit(Action::Primary)) launch();
    state.remainder += std::min<uint32_t>(delta, 1000);
    while (state.remainder >= 5 && state.status == Status::Running) {
        state.remainder -= 5; tick(controls);
    }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 5;
    reduce(state.flash_ms);
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) {
            state.level = uint16_t(std::min(1000, int(state.level) + 1));
            prepare_level();
        }
        return;
    }
    reduce(state.expanded_ms); reduce(state.slow_ms);
    int direction = int(controls.down(Action::Right)) - int(controls.down(Action::Left));
    state.paddle = std::clamp(state.paddle + direction * 3.1f, paddle_width() / 2, Width - paddle_width() / 2);
    if (state.waiting) { state.balls[0].x = state.paddle; return; }
    for (auto &b : state.balls) if (b.active) {
        normalize(b);
        float old_x = b.x, old_y = b.y;
        b.x += b.vx * .005f; b.y += b.vy * .005f;
        if (b.x < Radius) { b.x = Radius; b.vx = std::abs(b.vx); sound_events |= Move; }
        if (b.x > Width - Radius) { b.x = Width - Radius; b.vx = -std::abs(b.vx); sound_events |= Move; }
        if (b.y < Radius) { b.y = Radius; b.vy = std::abs(b.vy); sound_events |= Move; }
        float top = PaddleY - PaddleHeight / 2;
        if (b.vy > 0 && old_y + Radius <= top && b.y + Radius >= top &&
            circle_rect(b.x, b.y, state.paddle - paddle_width() / 2, top, paddle_width(), PaddleHeight)) {
            float offset = std::clamp((b.x - state.paddle) / (paddle_width() / 2), -1.f, 1.f);
            b.vx = std::clamp(offset * .88f + direction * .10f, -.92f, .92f) * ball_speed();
            b.vy = -std::sqrt(ball_speed() * ball_speed() - b.vx * b.vx);
            b.y = top - Radius - .05f;
            state.flash_ms = 100; sound_events |= Move;
        }
        brick_collision(b, old_x, old_y);
        if (state.status != Status::Running || state.transition) return;
        if (b.y > Height + Radius) b.active = 0;
    }
    for (auto &p : state.pickups) if (p.active) {
        p.y += 1.f;
        if (p.y > Height + 16) { p.active = 0; continue; }
        if (std::abs(p.x - state.paddle) < paddle_width() / 2 + 11 && std::abs(p.y - PaddleY) < PaddleHeight / 2 + 11) {
            p.active = 0; state.score += 100; apply(Power(p.type));
        }
    }
    unsigned alive = 0;
    for (const auto &b : state.balls) alive += b.active;
    if (!alive) {
        if (state.options.mode != 3) --state.lives;
        sound_events |= Crash;
        if (!state.lives) finish(Status::Lost);
        else { state.expanded_ms = state.slow_ms = 0; serve(); }
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value || unsigned(s.status) > 3 ||
        !s.level || s.level > 1000 || s.lives > 3 || s.waiting > 1 || s.expanded_ms > 15000 ||
        s.slow_ms > 10000 || s.transition > 1200 || s.flash_ms > 100 || s.remainder >= 5) return false;
    auto finite = [](float x, float low, float high) { return std::isfinite(x) && x >= low && x <= high; };
    float half = (PaddleWidths[s.options.difficulty] + (s.expanded_ms ? 60 : 0)) / 2;
    if (!finite(s.paddle, half, Width - half)) return false;
    unsigned remaining = 0, balls = 0;
    for (auto hp : s.bricks) { if (hp > 3) return false; remaining += hp != 0; }
    if (remaining != s.remaining || (s.transition && (remaining || s.waiting))) return false;
    for (const auto &b : s.balls) {
        if (b.active > 1 || !finite(b.x, -12, Width + 12) || !finite(b.y, -12, Height + 20) ||
            !finite(b.vx, -650, 650) || !finite(b.vy, -650, 650)) return false;
        if (b.active && !s.waiting && (std::hypot(b.vx, b.vy) < 100 || std::hypot(b.vx, b.vy) > 651)) return false;
        balls += b.active;
    }
    if ((s.waiting && (balls != 1 || !s.balls[0].active || s.balls[0].vx || s.balls[0].vy)) ||
        ((s.status == Status::Running || s.status == Status::Paused) &&
         (!s.lives || (!s.transition && (!balls || !remaining))))) return false;
    for (const auto &p : s.pickups)
        if (p.active > 1 || p.type > 2 || !finite(p.x, -16, Width + 16) || !finite(p.y, -16, Height + 20)) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0; return true;
}
}
