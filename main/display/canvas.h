#pragma once
#include <cstdint>
namespace arcade {
enum class Font { Small, Body, Title, Pixel };
class Canvas {
  public:
    virtual ~Canvas() = default;
    virtual void rect(int x, int y, int width, int height, uint32_t color, int radius = 0,
                      uint8_t opacity = 255) = 0;
    virtual void text(int x, int y, const char *text, uint32_t color, Font font = Font::Body) = 0;
};
namespace color {
constexpr uint32_t Bg = 0x0b1020, Panel = 0x19233c, Text = 0xf1f5ff, Muted = 0x97a6c5,
                   Gold = 0xffd166, Cyan = 0x27d9ef;
}
} // namespace arcade
