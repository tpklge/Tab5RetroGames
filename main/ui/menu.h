#pragma once
#include "lvgl.h"
#include <functional>
#include <string>
#include <vector>
namespace arcade {
struct MenuItem {
    MenuItem(std::string label, std::function<void()> activate = {},
             std::function<void(int)> adjust = {}, int icon = -1)
        : label(std::move(label)), activate(std::move(activate)), adjust(std::move(adjust)),
          icon(icon) {}
    std::string label;
    std::function<void()> activate;
    std::function<void(int)> adjust;
    int icon = -1;
};
class Menu {
  public:
    void show(const char *title, const std::string &description, std::vector<MenuItem> items,
              unsigned selected, uint8_t theme);
    void select(unsigned index);
    void activate();
    void adjust(int direction);
    void close();
    unsigned count() const { return unsigned(items.size()); }
    unsigned selection() const { return selected; }
    lv_obj_t *root() const { return container; }

  private:
    lv_obj_t *container = nullptr;
    std::vector<MenuItem> items;
    std::vector<lv_obj_t *> buttons;
    unsigned selected = 0;
};
} // namespace arcade
