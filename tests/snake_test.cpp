#include "model.h"
#include <cassert>
#include <cstdio>
using namespace arcade;
using namespace arcade::snake;
int main() {
    Model game;
    game.begin({0, 1, 0, 0, 12});
    game.turn(3);
    assert(!game.state.pending_count); // No reversal.
    auto cell = game.segment(0);
    game.state.food[0] = {uint16_t(cell + 1), 0, 1, 0};
    game.step(game.interval(), {});
    assert(game.state.length == 6 && game.state.score == 10);
    for (int type = 1; type < 6; ++type) {
        game.begin({0, 0, 1, 0, 55});
        auto head = game.segment(0);
        game.state.food[1] = {uint16_t(head + 1), uint8_t(type), 1, DefaultFood[type].lifetime_ms};
        game.step(game.interval(), {});
        assert(game.state.score == DefaultFood[type].points);
        if (type == 3)
            assert(game.state.slow_ms == 6000);
        if (type == 4)
            assert(game.state.multiplier_ms == 8000);
    }
    game.begin({0, 0, 1, 0, 99});
    game.state.body[0] = uint16_t(2 * Width + Width - 1);
    game.state.previous_head = game.state.body[0];
    game.step(game.interval(), {});
    assert(game.segment(0) == 2 * Width);
    game.begin({0, 0, 0, 0, 99});
    game.state.body[0] = uint16_t(2 * Width + Width - 1);
    game.step(game.interval(), {});
    assert(game.state.status == Status::Lost);
    for (int map = 0; map < 4; ++map) {
        game.begin({0, 3, uint8_t(map), 0, 22});
        for (int i = 0; i < 100; ++i)
            for (int slot = 0; slot < 3; ++slot) {
                game.state.food[slot].active = 0;
                assert(game.spawn_food(slot, uint8_t(slot)));
                auto pos = game.state.food[slot].cell;
                assert(!game.state.walls[pos] && !game.occupied(pos));
            }
    }
    game.begin({0, 0, 1, 0, 19});
    game.state.food[1] = {uint16_t(18 * Width), 5, 1, 50};
    game.step(51, {});
    assert(!game.state.food[1].active);
    auto bytes = game.save();
    Model copy;
    assert(copy.load(bytes));
    assert(copy.save() == bytes);
    game.step(16, {});
    copy.step(16, {});
    assert(game.save() == copy.save());
    auto bad = game.state;
    bad.body[bad.head] = Capacity;
    assert(!copy.load(snapshot(bad)));
    game.begin({0, 0, 0, 0, 19});
    game.state.food[0].active = 0;
    game.state.length = 4;
    game.state.direction = 0;
    game.state.body[0] = 2 * Width + 2;
    game.state.body[1] = 2 * Width + 3;
    game.state.body[2] = 1 * Width + 3;
    game.state.body[3] = 1 * Width + 2;
    game.step(game.interval(), {});
    assert(game.state.status == Status::Running);
    // The same move is a collision when growth prevents the tail vacating.
    game.begin({0, 0, 0, 0, 19});
    game.state.food[0].active = 0;
    game.state.length = 4;
    game.state.direction = 0;
    game.state.growing = 1;
    game.state.body[0] = 2 * Width + 2;
    game.state.body[1] = 2 * Width + 3;
    game.state.body[2] = 1 * Width + 3;
    game.state.body[3] = 1 * Width + 2;
    game.step(game.interval(), {});
    assert(game.state.status == Status::Lost);
    game.begin({2, 0, 0, 0, 19});
    game.step(120000, {});
    assert(game.state.status == Status::Won);
    game.begin({0, 0, 1, 0, 19});
    game.state.multiplier_ms = 8000;
    game.state.food[0] = {uint16_t(game.segment(0) + 1), 0, 1, 0};
    game.step(game.interval(), {});
    assert(game.state.score == 20);
    puts("PASS Snake: reversal guard, food growth/scores/effects, wrap, walls, safe food in every "
         "map, expiration, validated deterministic save/load");
}
