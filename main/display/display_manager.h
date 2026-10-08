#pragma once
#include "esp_err.h"
#include "m5_tab5_component.h"
namespace arcade::display {
esp_err_t init();
void brightness(uint8_t percent);
m5::tab5::m5tab5_component &board();
uint32_t refreshes();
} // namespace arcade::display
