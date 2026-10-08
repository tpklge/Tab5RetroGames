#pragma once
#include <cstdint>
namespace arcade::block {
constexpr int Columns = 10, Rows = 24, Hidden = 4;
constexpr uint32_t LockMs = 500, ClearMs = 140;
constexpr uint32_t Colors[8] = {0,        0x29dbe8, 0xffda42, 0xb469f5,
                                0x44e17e, 0xff5367, 0x438cff, 0xff9b40};
enum Piece : int8_t { I, O, T, S, Z, J, L, None = -1 };
} // namespace arcade::block
