#pragma once
namespace arcade::bomberman {
constexpr int Width = 15, Height = 11, Cells = Width * Height, Cell = 44;
constexpr int Start = Width + 1, Exit = (Height - 2) * Width + Width - 2;
constexpr int MaxBombs = 5, MaxEnemies = 8, CampaignLevels = 5;
constexpr unsigned FuseMs = 2200, FlameMs = 500;
}
