#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::frogger {
namespace {
float wrap(float x, float period) { x = std::fmod(x, period); return x < 0 ? x + period : x; }
void reduce(uint32_t &timer) { timer = timer > 10 ? timer - 10 : 0; }
int requested(const Controls &c, bool held) {
    const Action actions[] = {Action::Up, Action::Right, Action::Down, Action::Left};
    for (int i = 0; i < 4; ++i) if (held ? c.down(actions[i]) : c.hit(actions[i])) return i;
    return 4;
}
}
uint32_t Model::crossing_time() const {
    return uint32_t(std::max(20000, 45000 - state.options.difficulty * 5000 - std::min(10000, int(state.level - 1) * 1500)));
}
bool Model::supported(unsigned lane, float x) const {
    if (lane >= 4) return false;
    auto &l = state.lanes[lane];
    float phase = wrap(x - l.offset, float(l.period));
    return phase >= 12 && phase <= l.length - 12;
}
bool Model::traffic(unsigned lane, float x) const {
    if (lane < 4 || lane >= LaneCount) return false;
    const auto &l = state.lanes[lane];
    float phase = wrap(x - l.offset, float(l.period));
    return phase < l.length + 14 || phase > l.period - 14;
}
void Model::respawn() {
    state.x = state.previous_x = Width / 2;
    state.row = state.previous_row = state.best_row = StartRow;
    state.queued = 4; state.cooldown = 0; state.remaining_ms = crossing_time();
}
void Model::prepare() {
    state.homes = 0; state.reason = 0; state.transition = state.respawn_ms = 0;
    float progress = std::min(2.5f, 1.f + (state.level - 1) * .10f);
    for (int i = 0; i < LaneCount; ++i) {
        auto &l = state.lanes[i];
        l.length = uint16_t(i < 4 ? 170 + (i % 3) * 18 : 60 + (i % 3) * 24);
        l.period = uint16_t(l.length + (i < 4 ? 88 : 180 - state.options.difficulty * 12));
        float base = i < 4 ? 42.f + i * 9 : 105.f + (i - 4) * 19;
        l.speed = base * (1 + state.options.difficulty * .16f) * progress * (i % 2 ? -1.f : 1.f);
        l.offset = float(state.random.range(l.period));
    }
    respawn();
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{}; state.options = options_valid(options) ? options : Options{};
    state.random.value = state.options.seed; sound_events = 0; prepare();
}
void Model::finish(Status status) { state.status = status; state.remainder %= 10; sound_events |= GameOver; }
void Model::die(uint8_t reason) {
    state.reason = reason; sound_events |= Crash;
    if (state.options.mode != 3) --state.lives;
    if (!state.lives) finish(Status::Lost);
    else { respawn(); state.respawn_ms = 700; }
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running) return;
    int wanted = requested(controls, false);
    if (wanted != 4 && !state.respawn_ms && !state.transition) state.queued = uint8_t(wanted);
    state.remainder += std::min<uint32_t>(delta, 1000);
    while (state.remainder >= 10 && state.status == Status::Running) {
        state.remainder -= 10; tick(controls);
    }
}
void Model::tick(const Controls &controls) {
    state.elapsed += 10;
    if (state.options.mode == 2 && state.elapsed >= 180000) { finish(Status::Won); return; }
    if (state.transition) {
        reduce(state.transition);
        if (!state.transition) { state.level = uint16_t(std::min(1000, int(state.level) + 1)); prepare(); sound_events |= Level; }
        return;
    }
    for (auto &l : state.lanes) l.offset = wrap(l.offset + l.speed * .01f, float(l.period));
    if (state.respawn_ms) { reduce(state.respawn_ms); return; }
    reduce(state.cooldown);
    if (state.row >= 1 && state.row <= 4) state.x += state.lanes[state.row - 1].speed * .01f;
    if (state.remaining_ms <= 10) { die(3); return; }
    state.remaining_ms -= 10;
    if (!state.cooldown) {
        int direction = state.queued != 4 ? state.queued : requested(controls, true);
        if (direction != 4) {
            state.queued = 4; state.previous_x = state.x; state.previous_row = state.row;
            if (direction == 0 && state.row) --state.row;
            if (direction == 2 && state.row < StartRow) ++state.row;
            if (direction == 1) state.x = std::min(Width - 18.f, state.x + CellWidth);
            if (direction == 3) state.x = std::max(18.f, state.x - CellWidth);
            state.cooldown = 140; sound_events |= Move;
            if (state.row < state.best_row) { state.score += 10 * (state.best_row - state.row); state.best_row = state.row; }
        }
    }
    if (state.x < 16 || state.x > Width - 16) { die(2); return; }
    if (!state.row) {
        int home = -1;
        for (int i = 0; i < HomeCount; ++i) if (std::abs(state.x - HomeX[i]) <= 18) home = i;
        if (home < 0 || (state.homes & (1 << home))) { die(2); return; }
        state.reason = 0;
        state.homes |= uint8_t(1 << home); ++state.crossings;
        state.score += 200 + state.remaining_ms / 100; sound_events |= Bonus;
        if (state.homes == 31) {
            state.score += 1000;
            if (state.options.mode == 0 && state.level >= 3) finish(Status::Won);
            else { state.transition = 1200; respawn(); }
        } else { respawn(); state.respawn_ms = 400; }
        return;
    }
    if (state.row >= 1 && state.row <= 4 && !supported(state.row - 1, state.x)) { die(2); return; }
    if (state.row >= 6 && state.row <= 10 && traffic(state.row - 2, state.x)) die(1);
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State s;
    if (!decode(bytes, s) || !options_valid(s.options) || !s.random.value || unsigned(s.status) > 3 ||
        !s.level || s.level > 1000 || s.lives > 3 || s.row > StartRow || s.previous_row > StartRow ||
        s.best_row > StartRow || s.homes > 31 || s.queued > 4 || s.reason > 3 ||
        s.cooldown > 140 || s.respawn_ms > 700 || s.transition > 1200 || s.remainder >= 10 ||
        s.remaining_ms > 45000 || ((s.status == Status::Running || s.status == Status::Paused) && !s.lives)) return false;
    if (!std::isfinite(s.x) || s.x < 0 || s.x > Width || !std::isfinite(s.previous_x) ||
        s.previous_x < 0 || s.previous_x > Width) return false;
    for (const auto &l : s.lanes)
        if (!std::isfinite(l.offset) || !std::isfinite(l.speed) || std::abs(l.speed) > 800 ||
            l.length < 50 || l.length > 220 || l.period < l.length + 40 || l.period > 500 ||
            l.offset < 0 || l.offset >= l.period) return false;
    std::memcpy(&state, &s, sizeof(state)); sound_events = 0; return true;
}
}
