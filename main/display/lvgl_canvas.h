#pragma once
#include "canvas.h"
#include "lvgl.h"
namespace arcade {
class LvglCanvas : public Canvas {
  public:
    explicit LvglCanvas(lv_layer_t *layer) : layer(layer) {}
    void rect(int x, int y, int width, int height, uint32_t color, int radius = 0,
              uint8_t opacity = 255) override;
    void text(int x, int y, const char *text, uint32_t color, Font font = Font::Body) override;

  private:
    lv_layer_t *layer;
};
} // namespace arcade
