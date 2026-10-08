#include "model.h"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace arcade;
using namespace arcade::racer;
int main() {
    Model game;
    game.begin({0, 0, 0, 0, 2});
    Controls controls;
    controls.held[size_t(Action::Up)] = 1;
    controls.held[size_t(Action::Left)] = 1;
    game.step(100, controls);
    assert(game.state.speed > 110 && game.state.x < 180 && game.state.distance > 0);
    float speed = game.state.speed;
    controls = {};
    controls.held[size_t(Action::Down)] = 1;
    game.step(100, controls);
    assert(game.state.speed < speed);
    controls = {};
    controls.held[size_t(Action::Primary)] = 1;
    game.step(100, controls);
    assert(game.state.turbo < 100 && game.state.boosting);
    controls = {};
    game.state.x = 180;
    game.state.traffic[0] = {180, PlayerY, game.state.speed, 1, 0, 0};
    game.step(16, controls);
    assert(game.state.health == 65 && game.state.invulnerable);
    game.step(16, controls);
    assert(game.state.health == 65);
    game.state.traffic[0] = {60, PlayerY + CarHeight + 1, 0, 1, 1, 0};
    game.step(16, controls);
    assert(game.state.overtakes == 1 && game.state.score >= 100);
    auto bytes = game.save();
    Model copy;
    assert(copy.load(bytes) && copy.save() == bytes);
    game.step(16, controls);
    copy.step(16, controls);
    assert(game.save() == copy.save());
    auto bad = game.state;
    bad.x = std::nanf("");
    assert(!copy.load(snapshot(bad)));
    Model blue, yellow, green, purple;
    blue.begin({0, 0, 0, 1, 1});
    yellow.begin({0, 0, 0, 3, 1});
    green.begin({0, 0, 0, 2, 1});
    purple.begin({0, 0, 0, 4, 1});
    controls = {};
    controls.held[size_t(Action::Up)] = 1;
    blue.step(100, controls);
    yellow.step(100, controls);
    assert(yellow.state.speed > blue.state.speed);
    assert(purple.state.health == 160 && Vehicles[2].steering > Vehicles[0].steering &&
           Vehicles[1].maximum > Vehicles[0].maximum);
    game.begin({1, 0, 0, 0, 1});
    game.state.elapsed = 59990;
    game.step(16, {});
    assert(game.state.status == Status::Lost);
    game.begin({1, 0, 0, 0, 1});
    game.state.distance = 3000;
    game.step(16, {});
    assert(game.state.status == Status::Won);
    game.begin({2, 3, 0, 0, 91});
    assert(game.spawn());
    unsigned first_lane = 0;
    for (const auto &car : game.state.traffic)
        if (car.active)
            first_lane = unsigned(car.x / 120);
    for (int attempt = 0; attempt < 30; ++attempt)
        game.spawn();
    unsigned count = 0;
    bool lanes[3]{};
    for (const auto &car : game.state.traffic)
        if (car.active) {
            ++count;
            lanes[unsigned(car.x / 120)] = true;
        }
    assert(count == 2 && lanes[first_lane] && !game.spawn());
    game.begin({3, 0, 0, 4, 91});
    game.state.distance = 999;
    game.state.health = 100;
    game.state.turbo = 30;
    game.step(100, {});
    assert(game.state.health == 115 && game.state.turbo > 50 && game.state.checkpoint == 2);
    puts("PASS Racer: simultaneous steering/acceleration, braking, turbo, collision immunity, "
         "passing score, real vehicle differences, timer, validated deterministic save/load");
}
