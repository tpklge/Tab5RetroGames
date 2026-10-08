#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
#include <type_traits>
namespace arcade {
enum class GameId : uint8_t { BlockDrop, Snake, Racer };
enum class Status : uint8_t { Running, Paused, Lost, Won };
enum class Action : uint8_t { Left, Right, Up, Down, Primary, CounterRotate, Hold, Count };
struct Controls {
    std::array<uint8_t, size_t(Action::Count)> press{}, held{};
    bool hit(Action action) const { return press[size_t(action)]; }
    bool down(Action action) const { return held[size_t(action)]; }
};
struct Options {
    uint8_t mode = 0, difficulty = 1, map = 0, vehicle = 0;
    uint32_t seed = 1;
};
struct Result {
    uint32_t score = 0, time_ms = 0, distance = 0;
    uint16_t level = 1;
    bool won = false;
};
enum Sound : uint32_t {
    Move = 1,
    Rotate = 2,
    Drop = 4,
    Line = 8,
    Combo = 16,
    Eat = 32,
    Special = 64,
    Bonus = 128,
    Crash = 256,
    Overtake = 512,
    Turbo = 1024,
    GameOver = 2048,
    Level = 4096
};
struct Random {
    uint32_t value = 1;
    uint32_t next() {
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 5;
        return value;
    }
    uint32_t range(uint32_t bound) { return bound ? next() % bound : 0; }
};
// Fixed-layout snapshots are schema-versioned, size-checked and CRC-protected
// by storage. No pointers or native-sized integers may be stored in a state.
template <class T> std::vector<uint8_t> snapshot(const T &state) {
    static_assert(std::is_trivially_copyable<T>::value, "Save state must be plain data");
    std::vector<uint8_t> bytes(sizeof(T));
    std::memcpy(bytes.data(), &state, sizeof(T));
    return bytes;
}
template <class T> bool decode(const std::vector<uint8_t> &bytes, T &state) {
    if (bytes.size() != sizeof(T))
        return false;
    std::memcpy(&state, bytes.data(), sizeof(T));
    return true;
}
inline bool options_valid(const Options &options) {
    return options.mode < 4 && options.difficulty < 4 && options.map < 4 && options.vehicle < 5 &&
           options.seed != 0;
}
} // namespace arcade
