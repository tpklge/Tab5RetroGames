#pragma once
namespace arcade::pacman {
constexpr int Width = 25, Height = 23, Cells = Width * Height, Cell = 24, GhostCount = 4;
constexpr int TunnelRow = 11, Start = 21 * Width + 12;
constexpr int Homes[GhostCount] = {11 * Width + 12, 11 * Width + 11, 11 * Width + 13, 9 * Width + 12};
constexpr int Corners[GhostCount] = {Width + 1, Width + 23, 21 * Width + 1, 21 * Width + 23};
constexpr const char *MapNames[] = {"CIRCUITO AZUL", "GALERIAS VIOLETA", "JARDIM ESMERALDA"};
constexpr unsigned GhostColors[] = {0xff647c, 0xffa3d8, 0x5de1df, 0xffb36a};
}
