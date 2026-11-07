#pragma once
#include "esp_err.h"
using sd_pwr_ctrl_handle_t = void *;
struct sd_pwr_ctrl_ldo_config_t { int ldo_chan_id; };
esp_err_t sd_pwr_ctrl_new_on_chip_ldo(const sd_pwr_ctrl_ldo_config_t *, sd_pwr_ctrl_handle_t *);
