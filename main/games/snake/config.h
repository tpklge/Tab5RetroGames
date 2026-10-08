#pragma once
#include <cstdint>
namespace arcade::snake {
constexpr int Width = 32, Height = 22, Capacity = Width * Height;
struct FoodRule {
    uint16_t points, growth;
    uint32_t lifetime_ms;
};
constexpr FoodRule DefaultFood[6] = {{10, 1, 0},    {30, 3, 10000}, {75, 5, 7000},
                                     {20, 0, 8000}, {20, 0, 8000},  {150, 10, 5000}};
constexpr uint32_t FoodColor[6] = {0xff465e, 0xffd166, 0xba71ff, 0x44bfff, 0x48e895, 0xffed76};
constexpr uint32_t Speed[4] = {210, 150, 100, 70};
} // namespace arcade::snake
