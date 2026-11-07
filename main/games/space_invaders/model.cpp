#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::invaders {
namespace {
void reduce(uint32_t &timer) { timer = timer > 10 ? timer - 10 : 0; }
bool near(float a, float b, float extent) { return std::abs(a - b) < extent; }
}
void Model::begin(Options options) {
    state = State{};
    state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed;
    sound_events = 0;
    wave();
}
void Model::wave() {
    std::fill_n(state.aliens, Count, uint8_t(1));
    std::fill_n(state.shields, 4 * ShieldRows * ShieldColumns, uint8_t(1));
    for (int b = 0; b < 4; ++b)
        for (int r = 2; r < 4; ++r)
            for (int c = 3; c < 5; ++c)
                state.shields[b * 32 + r * 8 + c] = 0;
    for (auto &shot : state.shots) shot = Shot{};
    state.formation_x = 90;
    state.formation_y = 65 + std::min(100, int(state.level - 1) * 10);
    state.direction = 1;
    state.march = state.enemy_clock = state.wave_delay = 0;
    state.invulnerable = 1500;
}
void Model::fire(float x, float y, bool enemy) {
    for (auto &shot : state.shots) if (!shot.active) {
        shot = {x, y, 1, uint8_t(enemy)};
        if (!enemy) sound_events |= Drop;
        return;
    }
}
void Model::finish(Status status) { state.status = status; state.remainder %= 10; sound_events |= GameOver; }
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    // Fixed 10 ms simulation; preserve fractional time through saves.
    state.remainder += std::min<uint32_t>(delta, 1000);
    while (state.remainder >= 10 && state.status == Status::Running) {
        state.remainder -= 10;
        tick(controls);
    }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 10;
    reduce(state.cooldown); reduce(state.invulnerable);
    if (state.options.mode == 2 && state.elapsed >= 120000) { finish(Status::Won); return; }
    if (state.wave_delay) {
        reduce(state.wave_delay);
        if (!state.wave_delay) { state.level = std::min(10000, int(state.level) + 1); wave(); sound_events |= Level; }
        return;
    }
    state.player = std::clamp(state.player + 3.4f *
        (int(controls.down(Action::Right)) - int(controls.down(Action::Left))), 18.f, Width - 18.f);
    if ((controls.down(Action::Primary) || controls.hit(Action::Primary) || state.options.mode == 3) && !state.cooldown) {
        fire(state.player, PlayerY - 18, false); state.cooldown = 240;
    }
    unsigned alive = 0;
    for (auto alien : state.aliens) alive += alien;
    state.march += 10;
    const uint32_t interval = std::max(45, 570 - int(state.options.difficulty) * 85 -
                                      int(state.level - 1) * 20 - int(Count - alive) * 8);
    if (state.march >= interval) {
        state.march %= interval;
        bool edge = false;
        for (int i = 0; i < Count; ++i) if (state.aliens[i]) {
            float next = state.formation_x + (i % Columns) * 52 + state.direction * 12;
            edge |= next < 18 || next > Width - 18;
        }
        if (edge) { state.direction = -state.direction; state.formation_y += 22; }
        else state.formation_x += state.direction * 12;
    }
    state.enemy_clock += 10;
    if (state.enemy_clock >= uint32_t(std::max(200, 1000 - int(state.options.difficulty) * 170 - int(state.level - 1) * 35))) {
        state.enemy_clock = 0;
        int col = int(state.random.range(Columns));
        for (int row = Rows - 1; row >= 0; --row) if (state.aliens[row * Columns + col]) {
            fire(state.formation_x + col * 52, state.formation_y + row * 43 + 14, true); break;
        }
    }
    state.ufo_clock += 10;
    if (!state.ufo_active && state.ufo_clock >= 18000) {
        state.ufo_clock = 0; state.ufo_active = 1; state.ufo = -30;
    }
    if (state.ufo_active) { state.ufo += 1.2f; if (state.ufo > Width + 30) state.ufo_active = 0; }
    for (auto &shot : state.shots) if (shot.active) {
        shot.y += shot.enemy ? 2.4f + .4f * state.options.difficulty : -5.6f;
        if (shot.y < 0 || shot.y > Height) { shot.active = 0; continue; }
        for (int i = 0; i < 128 && shot.active; ++i) if (state.shields[i]) {
            const int b = i / 32, r = (i % 32) / 8, c = i % 8;
            if (near(shot.x, 80 + b * 165 + c * 8 + 4, 6) && near(shot.y, 451 + r * 8 + 4, 7)) {
                state.shields[i] = 0; shot.active = 0;
            }
        }
        if (!shot.active) continue;
        if (shot.enemy) {
            if (near(shot.x, state.player, 18) && near(shot.y, PlayerY, 16)) {
                shot.active = 0;
                if (!state.invulnerable) {
                    --state.lives; state.invulnerable = 1800; sound_events |= Crash;
                    if (!state.lives) { finish(Status::Lost); return; }
                }
            }
        } else {
            if (state.ufo_active && near(shot.x, state.ufo, 24) && near(shot.y, 25, 14)) {
                state.ufo_active = 0; shot.active = 0; state.score += 150; sound_events |= Bonus;
            }
            for (int i = 0; i < Count && shot.active; ++i) if (state.aliens[i] &&
                near(shot.x, state.formation_x + (i % Columns) * 52, 18) &&
                near(shot.y, state.formation_y + (i / Columns) * 43, 14)) {
                shot.active = 0; state.aliens[i] = 0;
                state.score += (3 - (i / Columns) / 2) * 10; sound_events |= Eat;
            }
        }
    }
    alive = 0;
    for (int i = 0; i < Count; ++i) if (state.aliens[i]) {
        ++alive;
        float y = state.formation_y + (i / Columns) * 43;
        if (y + 14 >= PlayerY - 16) { finish(Status::Lost); return; }
        for (int s = 0; s < 128; ++s)
            if (near(state.formation_x + (i % Columns) * 52, 84 + (s / 32) * 165 + (s % 8) * 8, 22) &&
                near(y, 455 + ((s % 32) / 8) * 8, 18)) state.shields[s] = 0;
    }
    if (!alive) {
        if (state.options.mode == 1 && state.level >= 5) finish(Status::Won);
        else state.wave_delay = 1200;
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value ||
        !std::isfinite(s.player) || s.player < 18 || s.player > Width - 18 ||
        !std::isfinite(s.formation_x) || s.formation_x < -500 || s.formation_x > Width ||
        !std::isfinite(s.formation_y) || s.formation_y < 0 || s.formation_y > Height ||
        !std::isfinite(s.ufo) || s.ufo < -40 || s.ufo > Width + 32 ||
        (s.direction != 1 && s.direction != -1) || s.lives > 3 || !s.level || s.level > 10000 ||
        s.ufo_active > 1 || unsigned(s.status) > 3 || s.remainder >= 10 ||
        s.cooldown > 240 || s.invulnerable > 1800 || s.wave_delay > 1200 ||
        (s.status == Status::Running && !s.lives)) return false;
    for (auto a : s.aliens) if (a > 1) return false;
    for (auto b : s.shields) if (b > 1) return false;
    for (const auto &shot : s.shots)
        if (shot.active > 1 || shot.enemy > 1 || !std::isfinite(shot.x) || !std::isfinite(shot.y) ||
            shot.x < -500 || shot.x > Width + 500 || shot.y < -10 || shot.y > Height + 10) return false;
    state = s; sound_events = 0; return true;
}
}
