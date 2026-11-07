#pragma once
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <cstddef>
struct esp_vfs_fat_sdmmc_mount_config_t {
    bool format_if_mount_failed = false;
    int max_files = 0;
    size_t allocation_unit_size = 0;
};
esp_err_t esp_vfs_fat_sdmmc_mount(const char *, const sdmmc_host_t *, const sdmmc_slot_config_t *,
                               const esp_vfs_fat_sdmmc_mount_config_t *, sdmmc_card_t **);
