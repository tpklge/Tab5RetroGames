#pragma once
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
constexpr int SDMMC_HOST_SLOT_0 = 0, SDMMC_FREQ_DEFAULT = 20000;
constexpr int GPIO_NUM_39 = 39, GPIO_NUM_40 = 40, GPIO_NUM_41 = 41,
              GPIO_NUM_42 = 42, GPIO_NUM_43 = 43, GPIO_NUM_44 = 44;
struct sdmmc_host_t { int slot = 1, max_freq_khz = 20000; sd_pwr_ctrl_handle_t pwr_ctrl_handle = nullptr; };
struct sdmmc_slot_config_t { int width = 4, clk = -1, cmd = -1, d0 = -1, d1 = -1, d2 = -1, d3 = -1; };
#define SDMMC_HOST_DEFAULT() sdmmc_host_t{}
#define SDMMC_SLOT_CONFIG_DEFAULT() sdmmc_slot_config_t{}
