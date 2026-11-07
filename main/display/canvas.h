#pragma once
#include <cstdint>
namespace arcade {
enum class Font { Small, Body, Title, Pixel };
class Canvas {
  public:
    virtual ~Canvas() = default;
    virtual void rect(int x, int y, int width, int height, uint32_t color, int radius = 0,
                      uint8_t opacity = 255) = 0;
    // Code-native masks and backgrounds can be batched by hardware renderers.
    virtual void sprite(int x, int y, const uint8_t *rows, unsigned height, int scale,
                        uint32_t paint, bool feet = false) {
        for (unsigned r = 0; r < height; ++r)
            for (int c = 0; c < 8;) {
                if (!(rows[r] & (1 << c))) { ++c; continue; }
                int first = c++;
                while (c < 8 && (rows[r] & (1 << c))) ++c;
                rect(x + first * scale, y + int(r) * scale, (c - first) * scale, scale, paint);
            }
        if (feet) {
            rect(x, y + int(height) * scale, scale, scale, paint);
            rect(x + 7 * scale, y + int(height) * scale, scale, scale, paint);
        }
    }
    virtual void checkerboard(int x, int y, int columns, int rows, int cell,
                              uint32_t first, uint32_t second) {
        rect(x, y, columns * cell, rows * cell, first);
        for (int r = 0; r < rows; ++r) for (int c = 0; c < columns; ++c)
            if ((r + c) % 2) rect(x + c * cell, y + r * cell, cell, cell, second);
    }
    virtual void dot_row(int x, int y, uint32_t mask, unsigned columns, int pitch, uint32_t paint) {
        for (unsigned col = 0; col < columns && col < 32; ++col)
            if (mask & (uint32_t(1) << col)) rect(x + int(col) * pitch + pitch / 2 - 2, y, 4, 4, paint);
    }
    virtual void line(int x1, int y1, int x2, int y2, uint32_t paint, int width = 2) {
        if (width < 1) return;
        int dx = x1 < x2 ? x2-x1 : x1-x2, sx = x1 < x2 ? 1 : -1;
        int dy = y1 < y2 ? y1-y2 : y2-y1, sy = y1 < y2 ? 1 : -1, error = dx + dy;
        for (;;) {
            rect(x1-width/2, y1-width/2, width, width, paint);
            if (x1 == x2 && y1 == y2) break;
            int twice = 2 * error;
            if (twice >= dy) { error += dy; x1 += sx; }
            if (twice <= dx) { error += dx; y1 += sy; }
        }
    }
    virtual void text(int x, int y, const char *text, uint32_t color, Font font = Font::Body) = 0;
};
namespace color {
constexpr uint32_t Bg = 0x0b1020, Panel = 0x19233c, Text = 0xf1f5ff, Muted = 0x97a6c5,
                   Gold = 0xffd166, Cyan = 0x27d9ef;
}
} // namespace arcade
