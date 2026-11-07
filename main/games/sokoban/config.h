#pragma once
namespace arcade::sokoban {
constexpr int Width = 11, Height = 9, Cells = Width * Height, Cell = 56;
constexpr int LevelsPerPack = 20, PackCount = 4, MaxUndo = 512;
constexpr unsigned NoCell = 255;
}
