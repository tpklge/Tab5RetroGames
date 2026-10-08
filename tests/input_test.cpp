#include "input_state.h"
#include <cassert>
#include <cstdio>
int main() {
    arcade::InputState input;
    using namespace arcade;
    assert(matrix_key(4, 10) == key::Left && matrix_key(3, 11) == key::Up);
    assert(matrix_key(5, 0) == 0 && matrix_key(0, 14) == 0);
    input.event(key::Left, true, 100);
    input.event(key::Up, true, 100);
    assert(input.held(key::Left) && input.held(key::Up));
    assert(input.repeat(key::Left, 100));
    input.end_frame();
    input.event(key::Left, true, 101);
    assert(!input.pressed(key::Left));
    assert(!input.repeat(key::Left, 349));
    assert(input.repeat(key::Left, 350));
    assert(!input.repeat(key::Left, 360));
    assert(input.repeat(key::Left, 415));
    input.event(key::Left, false, 416);
    assert(input.released(key::Left) && !input.held(key::Left));
    assert(input.held(key::Up));
    input.clear();
    input.event(key::Right, true, UINT32_MAX - 100);
    input.end_frame();
    assert(input.repeat(key::Right, 150));
    assert(key_character(4, false) == 'a' && key_character(4, true) == 'A');
    puts("PASS input: official matrix, independent simultaneous keys, edges, duplicate rejection, "
         "repeat and tick wrap");
}
