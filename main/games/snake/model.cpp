#include "model.h"
#include <algorithm>
namespace arcade::snake {
void Model::build_map() {
    if (state.options.map == 2) {
        for (int bar = 0; bar < 2 + state.options.difficulty; ++bar) {
            int y = 3 + bar * 4;
            for (int x = 5; x < 12 + bar * 2; ++x)
                state.walls[y * Width + x] = 1;
        }
    }
    if (state.options.map == 3)
        for (int y = 4; y < Height - 2; y += 5) {
            bool gap_left = (y / 5) % 2;
            for (int x = 1; x < Width - 1; ++x)
                if ((gap_left && x > 4) || (!gap_left && x < Width - 5))
                    state.walls[y * Width + x] = 1;
        }
}
void Model::begin(Options options) {
    std::fill_n(reinterpret_cast<uint8_t *>(&state), sizeof(state), 0);
    state = State{};
    state.options = options;
    state.random.value = options.seed ? options.seed : 1;
    if (state.options.mode == 3)
        state.options.map = 3;
    std::copy(std::begin(DefaultFood), std::end(DefaultFood), std::begin(state.rules));
    build_map();
    // The starting row is deliberately outside all obstacle/maze bars.
    for (unsigned index = 0; index < state.length; ++index)
        state.body[index] = uint16_t(2 * Width + 10 - index);
    state.previous_head = segment(0);
    state.previous_tail = segment(state.length - 1);
    sound_events = 0;
    spawn_food(0, 0);
}
bool Model::occupied(uint16_t cell) const {
    for (unsigned index = 0; index < state.length; ++index)
        if (segment(index) == cell)
            return true;
    return false;
}
bool Model::spawn_food(unsigned slot, uint8_t type) {
    if (slot >= 3 || type >= 6)
        return false;
    // Flood fill checks reachability, not only emptiness. Snake cells cannot be
    // crossed for spawning; its moving tail is allowed as a future exit.
    uint8_t visited[Capacity]{};
    uint16_t queue[Capacity];
    int count = 1, position = 0;
    queue[0] = segment(0);
    visited[queue[0]] = 1;
    while (position < count) {
        int cell = queue[position++], x = cell % Width, y = cell / Width;
        const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
        for (int direction = 0; direction < 4; ++direction) {
            int nx = x + dx[direction], ny = y + dy[direction];
            if (state.options.map == 1) {
                nx = (nx + Width) % Width;
                ny = (ny + Height) % Height;
            }
            if (nx < 0 || nx >= Width || ny < 0 || ny >= Height)
                continue;
            int next = ny * Width + nx;
            if (!visited[next] && !state.walls[next] &&
                (!occupied(next) || next == segment(state.length - 1))) {
                visited[next] = 1;
                queue[count++] = uint16_t(next);
            }
        }
    }
    uint32_t eligible = 0;
    uint16_t choice = 0;
    for (int cell = 0; cell < Capacity; ++cell) {
        if (!visited[cell] || state.walls[cell] || occupied(uint16_t(cell)))
            continue;
        bool used = false;
        for (unsigned i = 0; i < 3; ++i)
            if (i != slot && state.food[i].active && state.food[i].cell == cell)
                used = true;
        if (!used && state.random.range(++eligible) == 0)
            choice = uint16_t(cell);
    }
    if (!eligible) {
        state.food[slot].active = 0;
        return false;
    }
    state.food[slot] = {choice, type, 1, state.rules[type].lifetime_ms};
    return true;
}
void Model::turn(uint8_t direction) {
    uint8_t previous =
        state.pending_count ? state.pending[state.pending_count - 1] : state.direction;
    if (direction == previous || direction == (previous + 2) % 4 || state.pending_count >= 2)
        return;
    state.pending[state.pending_count++] = direction;
}
uint32_t Model::interval() const {
    uint32_t speed = Speed[state.options.difficulty];
    speed = std::max<uint32_t>(
        40, speed - std::min<uint32_t>(speed - 40, uint32_t(state.eaten) *
                                                       (state.options.mode == 1 ? 3u : 2u)));
    return state.slow_ms ? speed * 2 : speed;
}
void Model::finish(Status status) {
    state.status = status;
    sound_events |= GameOver;
}
void Model::advance() {
    if (state.pending_count) {
        state.direction = state.pending[0];
        state.pending[0] = state.pending[1];
        --state.pending_count;
    }
    int head = segment(0), x = head % Width, y = head / Width;
    const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
    x += dx[state.direction];
    y += dy[state.direction];
    if (state.options.map == 1) {
        x = (x + Width) % Width;
        y = (y + Height) % Height;
    }
    if (x < 0 || x >= Width || y < 0 || y >= Height) {
        finish(Status::Lost);
        return;
    }
    uint16_t next = uint16_t(y * Width + x);
    if (state.walls[next]) {
        finish(Status::Lost);
        return;
    }
    int food = -1;
    for (int i = 0; i < 3; ++i)
        if (state.food[i].active && state.food[i].cell == next)
            food = i;
    unsigned growth = state.growing + (food >= 0 ? state.rules[state.food[food].type].growth : 0);
    // Moving into the current tail is legal only when the tail will vacate.
    unsigned collision_length = state.length - (growth ? 0 : 1);
    for (unsigned i = 0; i < collision_length; ++i)
        if (segment(i) == next) {
            finish(Status::Lost);
            return;
        }
    state.previous_head = segment(0);
    state.previous_tail = segment(state.length - 1);
    state.head = (state.head + Capacity - 1) % Capacity;
    state.body[state.head] = next;
    if (growth && state.length < Capacity) {
        ++state.length;
        --growth;
    }
    state.growing = uint16_t(std::min(unsigned(Capacity - state.length), growth));
    if (food >= 0) {
        uint8_t type = state.food[food].type;
        state.score += state.rules[type].points * (state.multiplier_ms ? 2u : 1u);
        state.food[food].active = 0;
        ++state.eaten;
        state.flash_ms = 180;
        if (type == 3)
            state.slow_ms = 6000;
        if (type == 4)
            state.multiplier_ms = 8000;
        sound_events |= type == 0 ? Eat : type == 4 ? Bonus : Special;
        if (food == 0 && !spawn_food(0, 0)) {
            unsigned free = 0;
            for (int i = 0; i < Capacity; ++i)
                if (!state.walls[i] && !occupied(uint16_t(i)))
                    ++free;
            if (!free)
                finish(Status::Won); // Filled the entire playable map.
        }
    }
}
void Model::step(uint32_t delta, const Controls &controls) {
    if (state.status != Status::Running)
        return;
    if (controls.hit(Action::Up))
        turn(0);
    if (controls.hit(Action::Right))
        turn(1);
    if (controls.hit(Action::Down))
        turn(2);
    if (controls.hit(Action::Left))
        turn(3);
    state.elapsed += delta;
    auto decay = [delta](uint32_t &value) { value = value > delta ? value - delta : 0; };
    decay(state.slow_ms);
    decay(state.multiplier_ms);
    decay(state.flash_ms);
    for (auto &food : state.food)
        if (food.active && food.type) {
            if (food.remaining <= delta)
                food.active = 0;
            else
                food.remaining -= delta;
        }
    if (!state.food[0].active)
        spawn_food(0, 0);
    if (state.options.mode == 2 && state.elapsed >= 120000) {
        finish(Status::Won);
        return;
    }
    state.spawn_clock += delta;
    if (state.spawn_clock >= 5000) {
        state.spawn_clock %= 5000;
        for (unsigned slot = 1; slot < 3; ++slot)
            if (!state.food[slot].active) {
                uint32_t roll = state.random.range(20);
                uint8_t type = roll < 8 ? 1 : roll < 12 ? 3 : roll < 16 ? 4 : roll < 19 ? 2 : 5;
                spawn_food(slot, type);
                break;
            }
    }
    state.clock += delta;
    while (state.clock >= interval() && state.status == Status::Running) {
        state.clock -= interval();
        advance();
    }
}
bool Model::load(const std::vector<uint8_t> &bytes) {
    State restored;
    if (!decode(bytes, restored) || !options_valid(restored.options) || !restored.random.value ||
        restored.head >= Capacity || restored.length < 1 || restored.length > Capacity ||
        restored.growing > Capacity - restored.length || restored.direction > 3 ||
        restored.pending_count > 2 || restored.previous_head >= Capacity ||
        restored.previous_tail >= Capacity || unsigned(restored.status) > 3)
        return false;
    uint8_t seen[Capacity]{};
    for (unsigned i = 0; i < restored.length; ++i) {
        unsigned cell = restored.body[(restored.head + i) % Capacity];
        if (cell >= Capacity || seen[cell] || restored.walls[cell])
            return false;
        seen[cell] = 1;
    }
    for (auto wall : restored.walls)
        if (wall > 1)
            return false;
    for (auto direction : restored.pending)
        if (direction > 3)
            return false;
    for (const auto &food : restored.food)
        if (food.type > 5 || food.cell >= Capacity || food.active > 1 ||
            (food.active && (seen[food.cell] || restored.walls[food.cell])))
            return false;
    for (const auto &rule : restored.rules)
        if (rule.growth > 100 || rule.points > 10000 || rule.lifetime_ms > 60000)
            return false;
    std::memcpy(&state, &restored, sizeof(state));
    sound_events = 0;
    return true;
}
Result Model::result() const {
    return {state.score, state.elapsed, 0, uint16_t(1 + state.eaten / 5),
            state.status == Status::Won};
}
} // namespace arcade::snake
