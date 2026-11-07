#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::impact {
namespace {
void reduce(uint32_t &value) { value = value > 10 ? value - 10 : 0; }
bool near(float x, float y, float ox, float oy, float w, float h) {
    return std::abs(x - ox) < w && std::abs(y - oy) < h;
}
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{};
    state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed;
    sound_events = 0;
}
void Model::finish(Status status) {
    state.status = status; state.remainder %= 10; sound_events |= GameOver;
}
void Model::fire(float x, float y, float vx, float vy, bool enemy) {
    for (auto &s : state.shots) if (!s.active) {
        s = {x, y, vx, vy, 1, uint8_t(enemy)}; return;
    }
}
void Model::drop(float x, float y, uint8_t type) {
    for (auto &p : state.pickups) if (!p.active) { p = {x, y, 1, uint8_t(type % 3)}; return; }
}
bool Model::spawn() {
    for (auto &e : state.enemies) if (!e.active) {
        uint8_t type = uint8_t(state.random.range(3));
        float y = 42 + float(state.random.range(Height - 84));
        e = {Width + 25.f, y, y, 0, 700, 1, type, uint8_t(type == 2 ? 3 : 2)};
        return true;
    }
    return false;
}
void Model::start_boss() {
    state.boss_active = 1;
    state.boss_max = uint16_t(60 + std::min(500, int(state.level - 1) * 25) + state.options.difficulty * 15);
    state.boss_health = state.boss_max;
    state.boss_x = Width - 110;
    state.boss_y = Height / 2;
    state.boss_age = state.boss_clock = 0;
    for (auto &e : state.enemies) e.active = 0;
    for (auto &s : state.shots) if (s.enemy) s.active = 0;
    sound_events |= Level;
}
void Model::damage() {
    if (state.invulnerable) return;
    --state.lives; state.invulnerable = 1800; state.flash = 240;
    if (state.weapon > 1) --state.weapon;
    sound_events |= Crash;
    if (!state.lives) finish(Status::Lost);
}
void Model::kill(Enemy &e) {
    e.active = 0; state.score += 60 + e.type * 30; ++state.kills; sound_events |= Eat;
    if (state.kills % 5 == 0) drop(e.x, e.y, uint8_t((state.kills / 5 - 1) % 3));
}
void Model::hit_boss(unsigned damage) {
    state.boss_health = uint16_t(damage >= state.boss_health ? 0 : state.boss_health - damage);
    if (state.boss_health) return;
    state.boss_active = 0; state.score += 1500 * state.level; sound_events |= Bonus;
    for (auto &s : state.shots) s.active = 0;
    if ((state.options.mode == 0 || state.options.mode == 3) && state.level >= 3) finish(Status::Won);
    else state.transition = 1800;
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    state.remainder += std::min<uint32_t>(delta, 1000);
    // A bomb is an edge action, consumed once even when several ticks are due.
    bool bomb = controls.hit(Action::CounterRotate);
    while (state.remainder >= 10 && state.status == Status::Running) {
        state.remainder -= 10;
        Controls next = controls;
        next.press[size_t(Action::CounterRotate)] = uint8_t(bomb);
        tick(next); bomb = false;
    }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 10;
    reduce(state.cooldown); reduce(state.invulnerable); reduce(state.flash);
    state.scroll = std::fmod(state.scroll + 1.4f, 2240.f);
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) {
            state.level = uint16_t(std::min(1000, int(state.level) + 1));
            state.stage_time = state.spawn_clock = 0;
            state.invulnerable = 1500;
            state.bombs = std::min(5, state.bombs + 1);
            for (auto &p : state.pickups) p.active = 0;
        }
        return;
    }
    float dx = float(int(controls.down(Action::Right)) - int(controls.down(Action::Left)));
    float dy = float(int(controls.down(Action::Down)) - int(controls.down(Action::Up)));
    const float speed = dx && dy ? 2.55f : 3.6f;
    state.x = std::clamp(state.x + dx * speed, 24.f, Width - 24.f);
    state.y = std::clamp(state.y + dy * speed, 24.f, Height - 24.f);
    if (!state.cooldown && (controls.down(Action::Primary) || controls.hit(Action::Primary) || state.options.mode == 3)) {
        fire(state.x + 25, state.y, 670, 0, false);
        if (state.weapon >= 2) fire(state.x + 20, state.y - 9, 650, -65, false);
        if (state.weapon >= 3) fire(state.x + 20, state.y + 9, 650, 65, false);
        state.cooldown = 160; sound_events |= Drop;
    }
    if (controls.hit(Action::CounterRotate) && state.bombs) {
        --state.bombs; state.flash = 300; sound_events |= Combo;
        for (auto &s : state.shots) if (s.enemy) s.active = 0;
        for (auto &e : state.enemies) if (e.active) kill(e);
        if (state.boss_active) hit_boss(18);
        if (state.transition || state.status != Status::Running) return;
    }
    state.stage_time += 10;
    if (!state.boss_active && state.stage_time >= StageDuration) start_boss();
    if (!state.boss_active) {
        state.spawn_clock += 10;
        uint32_t interval = uint32_t(std::max(300, 1100 - state.options.difficulty * 160 - std::min(400, int(state.level - 1) * 70)));
        if (state.spawn_clock >= interval) { state.spawn_clock %= interval; spawn(); }
    }
    for (auto &e : state.enemies) if (e.active) {
        e.age += 10;
        e.x -= (e.type == 1 ? 2.7f : 1.65f) + state.options.difficulty * .2f;
        if (e.type == 0) e.y = e.origin_y + std::sin(e.age * .0035f) * 32;
        if (e.type == 1) e.y += std::clamp(state.y - e.y, -1.f, 1.f);
        reduce(e.cooldown);
        if (e.type != 1 && !e.cooldown) {
            float vx = state.x - e.x, vy = state.y - e.y;
            float length = std::max(1.f, std::sqrt(vx * vx + vy * vy));
            float speed = 190 + state.options.difficulty * 35;
            fire(e.x - 20, e.y, vx / length * speed, vy / length * speed, true);
            e.cooldown = 1500 - state.options.difficulty * 200;
        }
        if (near(state.x, state.y, e.x, e.y, 38, 27)) { e.active = 0; damage(); }
        if (e.x < -40) e.active = 0;
    }
    if (state.status != Status::Running) return;
    if (state.boss_active) {
        state.boss_age += 10; state.boss_clock += 10;
        unsigned type = (state.level - 1) % 3;
        state.boss_y = Height / 2 + std::sin(state.boss_age * .0015f) * (type == 1 ? 155 : 110);
        state.boss_x = Width - 120 - (type == 2 ? 45 * (1 + std::sin(state.boss_age * .002f)) : 0);
        uint32_t interval = state.boss_health * 2 < state.boss_max ? 450 : 750;
        interval -= state.options.difficulty * 60;
        if (state.boss_clock >= interval) {
            state.boss_clock = 0;
            float vx = state.x - state.boss_x, vy = state.y - state.boss_y;
            float length = std::max(1.f, std::sqrt(vx * vx + vy * vy));
            fire(state.boss_x - 50, state.boss_y, vx / length * 260, vy / length * 260, true);
            int spread = type == 0 ? 1 : type == 1 ? 2 : 3;
            for (int i = -spread; i <= spread; ++i)
                fire(state.boss_x - 50, state.boss_y, -250, i * 80.f, true);
        }
        if (near(state.x, state.y, state.boss_x, state.boss_y, 76, 65)) damage();
    }
    if (state.status != Status::Running) return;
    for (auto &s : state.shots) if (s.active) {
        s.x += s.vx * .01f; s.y += s.vy * .01f;
        if (s.x < -40 || s.x > Width + 40 || s.y < -40 || s.y > Height + 40) { s.active = 0; continue; }
        if (s.enemy) {
            if (near(s.x, s.y, state.x, state.y, 23, 16)) { s.active = 0; damage(); }
        } else {
            for (auto &e : state.enemies) if (s.active && e.active && near(s.x, s.y, e.x, e.y, 26, 23)) {
                s.active = 0; if (!--e.health) kill(e);
            }
            if (s.active && state.boss_active && near(s.x, s.y, state.boss_x, state.boss_y, 58, 55)) {
                s.active = 0; hit_boss(1);
                if (state.transition || state.status != Status::Running) return;
            }
        }
        if (state.status != Status::Running) return;
    }
    for (auto &p : state.pickups) if (p.active) {
        p.x -= 1.3f;
        if (p.x < -20) p.active = 0;
        if (near(state.x, state.y, p.x, p.y, 30, 28)) {
            p.active = 0; state.score += 100; sound_events |= Special;
            if (p.type == 0) state.weapon = std::min(3, state.weapon + 1);
            if (p.type == 1) state.lives = std::min(3, state.lives + 1);
            if (p.type == 2) state.bombs = std::min(5, state.bombs + 1);
        }
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value ||
        unsigned(s.status) > 3 || s.lives > 3 || (!s.lives && s.status == Status::Running) ||
        s.weapon < 1 || s.weapon > 3 || s.bombs > 5 || s.boss_active > 1 ||
        !s.level || s.level > 1000 || s.boss_max > 605 || s.boss_health > s.boss_max ||
        (s.boss_active && (!s.boss_health || !s.boss_max)) ||
        s.remainder >= 10 || s.cooldown > 160 || s.invulnerable > 1800 || s.transition > 1800 || s.flash > 300)
        return false;
    auto valid = [](float x, float low, float high) { return std::isfinite(x) && x >= low && x <= high; };
    if (!valid(s.x, 24, Width - 24) || !valid(s.y, 24, Height - 24) || !valid(s.scroll, 0, 2240) ||
        !valid(s.boss_x, 0, Width) || !valid(s.boss_y, 0, Height)) return false;
    for (const auto &e : s.enemies)
        if (!valid(e.x, -50, Width + 30) || !valid(e.y, 0, Height) || !valid(e.origin_y, 0, Height) ||
            e.active > 1 || e.type > 2 || e.health > 3 || (e.active && !e.health) || e.cooldown > 1500) return false;
    for (const auto &shot : s.shots)
        if (!valid(shot.x, -50, Width + 50) || !valid(shot.y, -50, Height + 50) ||
            !valid(shot.vx, -700, 700) || !valid(shot.vy, -700, 700) || shot.active > 1 || shot.enemy > 1) return false;
    for (const auto &p : s.pickups)
        if (!valid(p.x, -50, Width + 50) || !valid(p.y, 0, Height) || p.active > 1 || p.type > 2) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0; return true;
}
}
