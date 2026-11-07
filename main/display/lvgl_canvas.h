#pragma once
#include "canvas.h"
#include "lvgl.h"
namespace arcade {
class LvglCanvas : public Canvas {
  public:
    explicit LvglCanvas(lv_layer_t *layer) : layer(layer) {}
    void rect(int x, int y, int width, int height, uint32_t color, int radius = 0,
              uint8_t opacity = 255) override;
    void sprite(int x, int y, const uint8_t *rows, unsigned height, int scale,
                uint32_t paint, bool feet = false) override;
    void checkerboard(int x, int y, int columns, int rows, int cell,
                      uint32_t first, uint32_t second) override;
    void dot_row(int x, int y, uint32_t mask, unsigned columns, int pitch, uint32_t paint) override;
    void line(int x1, int y1, int x2, int y2, uint32_t paint, int width = 2) override;
    void text(int x, int y, const char *text, uint32_t color, Font font = Font::Body) override;

  private:
    lv_layer_t *layer;
};
} // namespace arcade
