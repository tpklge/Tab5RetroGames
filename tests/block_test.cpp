#include "model.h"
#include <cassert>
#include <set>
#include <cstdio>
using namespace arcade;
using namespace arcade::block;
int main() {
    Model game;
    game.begin({0, 0, 0, 0, 12345});
    std::set<int> bag;
    bag.insert(game.state.piece);
    for (int i = 0; i < 6; ++i)
        bag.insert(game.state.queue[i]);
    assert(bag.size() == 7);
    for (int piece = 0; piece < 7; ++piece) {
        game.state.piece = piece;
        game.state.x = 3;
        game.state.y = 10;
        game.state.rotation = 0;
        auto cells = game.cells(piece, 0);
        if (piece != O)
            for (int i = 0; i < 4; ++i)
                assert(game.rotate(1));
        assert(game.state.rotation == 0);
        auto again = game.cells(piece, 0);
        for (int i = 0; i < 4; ++i)
            assert(cells[i].x == again[i].x && cells[i].y == again[i].y);
    }
    game.begin({0, 0, 0, 0, 1});
    game.state.piece = T;
    game.state.rotation = 1;
    game.state.x = -1;
    game.state.y = 10;
    assert(!game.collision(-1, 10, 1, T));
    assert(game.rotate(-1));
    assert(game.state.x == 0); // SRS wall kick.
    game.begin({0, 0, 0, 0, 1});
    game.state.piece = I;
    game.state.rotation = 1;
    game.state.x = 2;
    game.state.y = 20;
    for (int y = 20; y < 24; ++y)
        for (int x = 0; x < 10; ++x)
            if (x != 4)
                game.state.board[y][x] = 7;
    game.lock_piece();
    assert(game.state.lines == 4 && game.state.score == 800 && game.state.clear_mask);
    game.step(ClearMs, {});
    assert(!game.state.clear_mask);
    for (auto &row : game.state.board)
        for (auto cell : row)
            assert(!cell);
    game.reserve();
    auto held = game.state.hold;
    game.reserve();
    assert(game.state.hold == held && game.state.hold_used);
    game.begin({0, 0, 0, 0, 1});
    game.state.piece = T;
    game.state.rotation = 0;
    game.state.x = 3;
    game.state.y = 20;
    game.state.last_rotation = 1;
    game.state.board[20][3] = 1;
    game.state.board[20][5] = 1;
    game.state.board[22][3] = 1;
    game.lock_piece();
    assert(game.state.spin == 2 && game.state.score == 400);
    auto bytes = game.save();
    Model copy;
    assert(copy.load(bytes));
    assert(copy.save() == bytes);
    game.step(16, {});
    copy.step(16, {});
    assert(game.save() == copy.save());
    bytes.pop_back();
    assert(!copy.load(bytes));
    game.begin({2, 0, 0, 0, 7});
    game.step(120000, {});
    assert(game.state.status == Status::Won);
    game.begin({3, 0, 0, 0, 7});
    game.state.lines = 100;
    assert(game.state.level == 1);
    // Consecutive four-line clears include B2B and the second combo bonus.
    game.begin({0, 0, 0, 0, 9});
    for (int clear = 0; clear < 2; ++clear) {
        game.state.piece = I;
        game.state.rotation = 1;
        game.state.x = 2;
        game.state.y = 20;
        for (int y = 20; y < Rows; ++y)
            for (int x = 0; x < Columns; ++x)
                game.state.board[y][x] = x == 4 ? 0 : 7;
        game.lock_piece();
        game.step(ClearMs, {});
    }
    assert(game.state.score == 2050 && game.state.combo == 1 && game.state.back_to_back);
    // A grounded piece waits 500 ms; lateral resets are capped at fifteen.
    game.begin({0, 0, 0, 0, 9});
    game.state.piece = O;
    game.state.y = 22;
    game.step(300, {});
    assert(game.state.lock == 300);
    assert(game.move(1, 0) && game.state.lock == 0);
    for (int move = 1; move < 15; ++move)
        assert(game.move(move % 2 ? -1 : 1, 0));
    assert(game.state.lock_resets == 15);
    game.step(499, {});
    assert(game.state.board[23][4] == 0);
    assert(game.move(-1, 0) && game.state.lock == 499);
    game.step(1, {});
    assert(game.state.board[23][4] != 0);
    game.begin({1, 0, 0, 0, 9});
    game.state.lines = 36;
    game.state.piece = I;
    game.state.rotation = 1;
    game.state.x = 2;
    game.state.y = 20;
    for (int y = 20; y < Rows; ++y)
        for (int x = 0; x < Columns; ++x)
            game.state.board[y][x] = x == 4 ? 0 : 7;
    game.lock_piece();
    game.step(ClearMs, {});
    assert(game.state.status == Status::Won);
    game.begin({0, 0, 0, 0, 9});
    game.state.piece = T;
    game.state.rotation = 0;
    game.state.x = 3;
    game.state.y = 20;
    game.state.last_rotation = 1;
    game.state.board[20][3] = 1;
    game.state.board[22][3] = 1;
    game.state.board[22][5] = 1;
    game.lock_piece();
    assert(game.state.spin == 1 && game.state.score == 100);
    auto invalid = game.state;
    invalid.bag[1] = invalid.bag[0];
    assert(!copy.load(snapshot(invalid)));
    puts("PASS Block Drop: 7-bag, all pieces, rotation cycles, SRS kicks, four-line clear, hold, "
         "T-spin, timed mode, save/load determinism");
}
