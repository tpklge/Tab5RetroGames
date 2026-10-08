#include "model.h"
#include <algorithm>
#include <cmath>
namespace arcade::racer {
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{};
    state.options = options;
    state.random.value = options.seed ? options.seed : 1;
    state.health = Vehicles[options.vehicle].health;
    sound_events = 0;
}
bool Model::spawn() {
    unsigned active = 0;
    for (const auto &car : state.traffic)
        active += car.active;
    if (active >= MaxTraffic)
        return false;
    int lane = int(state.random.range(3));
    float x = 60.f + 120.f * lane;
    bool blocked[3]{};
    for (const auto &car : state.traffic)
        if (car.active && car.y < 150) {
            int index = int(car.x / 120);
            if (index >= 0 && index < 3)
                blocked[index] = true;
        }
    if (blocked[lane] || (blocked[0] + blocked[1] + blocked[2]) >= 2)
        return false;
    for (auto &car : state.traffic)
        if (!car.active) {
            car = {x,
                   -90.f,
                   35.f + float(state.random.range(55)) + state.options.difficulty * 5.f,
                   1,
                   uint8_t(state.random.range(5)),
                   0};
            return true;
        }
    return false;
}
bool Model::collision(const Traffic &car) const {
    return std::abs(state.x - car.x) < CarWidth - 4 && std::abs(PlayerY - car.y) < CarHeight - 8;
}
void Model::finish(Status status) {
    state.status = status;
    sound_events |= GameOver;
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running)
        return;
    const auto &vehicle = Vehicles[state.options.vehicle];
    float dt = delta * .001f;
    state.elapsed += delta;
    state.invulnerable = state.invulnerable > delta ? state.invulnerable - delta : 0;
    state.flash_ms = state.flash_ms > delta ? state.flash_ms - delta : 0;
    state.previous_x = state.x;
    bool turbo = controls.down(Action::Primary) && state.turbo > 0;
    if (turbo && !state.boosting)
        sound_events |= Turbo;
    state.boosting = turbo;
    state.turbo = std::clamp(state.turbo + (turbo ? -42.f : 9.f) * dt, 0.f, 100.f);
    float acceleration = controls.down(Action::Down) ? -160.f
                         : controls.down(Action::Up) ? vehicle.acceleration
                         : state.speed > 110         ? -12.f
                                                     : 18.f;
    if (turbo)
        acceleration += 180;
    state.speed =
        std::clamp(state.speed + acceleration * dt, 25.f, vehicle.maximum * (turbo ? 1.35f : 1.f));
    int direction = int(controls.down(Action::Right)) - int(controls.down(Action::Left));
    state.x += direction * vehicle.steering * dt * (.55f + .45f * state.speed / vehicle.maximum);
    if (state.x < CarWidth / 2 + 4 || state.x > RoadWidth - CarWidth / 2 - 4) {
        state.x = std::clamp(state.x, CarWidth / 2 + 4, RoadWidth - CarWidth / 2 - 4);
        state.speed = std::max(25.f, state.speed - 50.f * dt);
    }
    state.distance += state.speed / 3.6f * dt;
    state.scroll = std::fmod(state.scroll + state.speed * .65f * dt, 120.f);
    state.level = uint16_t(1 + std::min(9999u, unsigned(state.distance / 800)));
    uint32_t interval = std::max<uint32_t>(
        330, 1700 - std::min<uint32_t>(1370, uint32_t(state.level) * 45 +
                                                 uint32_t(state.options.difficulty) * 230));
    if (state.options.mode == 2)
        interval = interval * 3 / 5;
    if (state.options.mode == 3)
        interval = interval * 5 / 4;
    state.spawn_clock += delta;
    if (state.spawn_clock >= interval) {
        state.spawn_clock %= interval;
        spawn();
    }
    for (auto &car : state.traffic)
        if (car.active) {
            car.y += (state.speed - car.speed) * .8f * dt;
            if (!state.invulnerable && collision(car)) {
                state.health = std::max(0.f, state.health - 35.f);
                state.invulnerable = 1500;
                state.flash_ms = 400;
                state.speed = std::max(25.f, state.speed * .45f);
                car.passed = 1;
                sound_events |= Crash;
                if (!state.health) {
                    finish(Status::Lost);
                    return;
                }
            }
            if (!car.passed && car.y > PlayerY + CarHeight) {
                car.passed = 1;
                ++state.overtakes;
                sound_events |= Overtake;
            }
            if (car.y > 650 || car.y < -160)
                car.active = 0;
        }
    if (state.distance >= state.checkpoint * 1000.f) {
        ++state.checkpoint;
        sound_events |= Level;
        if (state.options.mode == 3) {
            state.health = std::min(vehicle.health, state.health + 15);
            state.turbo = std::min(100.f, state.turbo + 20);
        }
    }
    state.score = uint32_t(state.distance) * (1 + state.options.difficulty) + state.overtakes * 100;
    if (state.options.mode == 1) {
        if (state.distance >= 3000)
            finish(Status::Won);
        else if (state.elapsed >= 60000)
            finish(Status::Lost);
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State restored;
    if (!decode(bytes, restored) || !options_valid(restored.options) || !restored.random.value ||
        !std::isfinite(restored.x) || restored.x < 18 || restored.x > 342 ||
        !std::isfinite(restored.previous_x) || restored.previous_x < 18 ||
        restored.previous_x > 342 || !std::isfinite(restored.speed) || restored.speed < 0 ||
        restored.speed > 400 || !std::isfinite(restored.distance) || restored.distance < 0 ||
        restored.distance > 10000000 || !std::isfinite(restored.health) || restored.health < 0 ||
        restored.health > 160 || !std::isfinite(restored.turbo) || restored.turbo < 0 ||
        restored.turbo > 100 || !std::isfinite(restored.scroll) || restored.scroll < 0 ||
        restored.scroll >= 120 || !restored.level || unsigned(restored.status) > 3 ||
        restored.boosting > 1)
        return false;
    for (const auto &car : restored.traffic)
        if (!std::isfinite(car.x) || !std::isfinite(car.y) || !std::isfinite(car.speed) ||
            car.active > 1 || car.color > 4 || car.passed > 1 || car.x < 0 || car.x > 360 ||
            car.y < -200 || car.y > 800 || car.speed < 0 || car.speed > 300)
            return false;
    std::memcpy(&state, &restored, sizeof(state));
    sound_events = 0;
    return true;
}
Result Model::result() const {
    return {state.score, state.elapsed, uint32_t(state.distance), state.level,
            state.status == Status::Won};
}
} // namespace arcade::racer
