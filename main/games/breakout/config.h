#pragma once
#include <cstdint>
namespace arcade::breakout {
constexpr int Width = 800, Height = 540, Columns = 10, Rows = 7, BrickCount = Columns * Rows;
constexpr int MaxBalls = 3, MaxPickups = 8, CampaignLevels = 6;
constexpr float Radius = 7, PaddleY = 500, PaddleHeight = 14;
constexpr int BrickLeft = 24, BrickTop = 60, BrickPitchX = 76, BrickPitchY = 31,
              BrickWidth = 70, BrickHeight = 24;
constexpr float InitialSpeed[] = {255, 310, 365, 420};
constexpr float PaddleWidths[] = {140, 118, 102, 90};
constexpr uint32_t BrickColors[] = {0xff6b8a, 0xffa45b, 0xffd166, 0x75dda0,
                                   0x56d6ed, 0x789dff, 0xc591ff};
enum class Power : uint8_t { Expand, Multiball, Slow };
constexpr const char *LayoutNames[] = {"PRIMEIRO CONTATO", "CONSTELACAO", "PORTAIS",
                                      "ZIGUEZAGUE", "DIAMANTE", "FORTALEZA"};
}
