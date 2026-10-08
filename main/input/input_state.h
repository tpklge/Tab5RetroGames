#pragma once
#include <array>
#include <cstdint>

namespace arcade {
namespace key {
constexpr uint8_t Enter = 0x28, Escape = 0x29, Backspace = 0x2a, Space = 0x2c;
constexpr uint8_t Right = 0x4f, Left = 0x50, Down = 0x51, Up = 0x52;
constexpr uint8_t P = 0x13, R = 0x15, M = 0x10, X = 0x1b, Z = 0x1d, C = 0x06;
} // namespace key
uint8_t matrix_key(uint8_t row, uint8_t col);
char key_character(uint8_t code, bool shift);
class InputState {
  public:
    void event(uint8_t code, bool pressed, uint32_t now);
    bool held(uint8_t code) const { return down[code]; }
    bool pressed(uint8_t code) const { return edges[code]; }
    bool released(uint8_t code) const { return ups[code]; }
    bool repeat(uint8_t code, uint32_t now, uint32_t delay = 250, uint32_t period = 65);
    void end_frame();
    void clear();

  private:
    std::array<uint8_t, 256> down{}, edges{}, ups{}, repeating{};
    std::array<uint32_t, 256> started{}, last_repeat{};
};
} // namespace arcade
