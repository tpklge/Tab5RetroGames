#pragma once
namespace arcade::frogger {
constexpr int Width = 720, Rows = 13, CellWidth = 48, CellHeight = 40, Height = Rows * CellHeight;
constexpr int LaneCount = 9, HomeCount = 5, StartRow = 12;
constexpr float HomeX[HomeCount] = {72, 216, 360, 504, 648};
}
