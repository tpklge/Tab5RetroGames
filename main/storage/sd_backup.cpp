#include "sd_backup.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
namespace arcade {
std::string backup_to_sd(Backend &storage, unsigned game_count) {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    slot.width = 4;
    slot.clk = GPIO_NUM_43;
    slot.cmd = GPIO_NUM_44;
    slot.d0 = GPIO_NUM_39;
    slot.d1 = GPIO_NUM_40;
    slot.d2 = GPIO_NUM_41;
    slot.d3 = GPIO_NUM_42;
    slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    esp_vfs_fat_sdmmc_mount_config_t config{};
    config.format_if_mount_failed = false;
    config.max_files = 3;
    sdmmc_card_t *card = nullptr;
    if (esp_vfs_fat_sdmmc_mount("/arcadesd", &host, &slot, &config, &card) != ESP_OK)
        return "microSD ausente ou FAT indisponivel. O armazenamento interno foi preservado.";
    const char *directory = "/arcadesd/pipoca5";
    const char *temporary = "/arcadesd/pipoca5/backup.tmp";
    bool ok = mkdir(directory, 0755) == 0 || errno == EEXIST;
    char filename[32] = "backup.bin";
    std::string destination;
    // FAT rename does not replace existing targets. Select a free destination
    // and keep earlier backups intact, including across power interruptions.
    for (unsigned serial = 0; ok && serial < 10000; ++serial) {
        if (serial)
            std::snprintf(filename, sizeof(filename), "backup-%04u.bin", serial);
        destination = std::string(directory) + "/" + filename;
        struct stat info{};
        if (stat(destination.c_str(), &info) != 0) {
            ok = errno == ENOENT;
            break;
        }
        if (serial == 9999)
            ok = false;
    }
    FILE *file = ok ? std::fopen(temporary, "wb") : nullptr;
    ok = file != nullptr;
    unsigned count = 0;
    auto write = [&](const void *data, size_t size) {
        if (ok && std::fwrite(data, 1, size, file) != size)
            ok = false;
    };
    // Stable little-endian archive; each entry contains its versioned CRC envelope.
    auto word = [&](uint32_t value) {
        uint8_t bytes[4];
        for (int i = 0; i < 4; ++i)
            bytes[i] = uint8_t(value >> (i * 8));
        write(bytes, sizeof(bytes));
    };
    word(0x35504241);
    word(1);
    auto copy = [&](const std::string &base) {
        for (char suffix : {'a', 'b'}) {
            std::string key = base + suffix;
            std::vector<uint8_t> bytes;
            if (!storage.read(key, bytes))
                continue;
            word(uint32_t(key.size()));
            word(uint32_t(bytes.size()));
            write(key.data(), key.size());
            write(bytes.data(), bytes.size());
            ++count;
        }
    };
    copy("config");
    for (unsigned game = 0; game < game_count; ++game) {
        copy("save" + std::to_string(game));
        for (unsigned mode = 0; mode < 4; ++mode)
            for (unsigned difficulty = 0; difficulty < 4; ++difficulty)
                copy("rank" + std::to_string(game) + std::to_string(mode) +
                     std::to_string(difficulty));
    }
    word(0);
    word(0);
    if (file) {
        if (std::fflush(file) != 0 || fsync(fileno(file)) != 0)
            ok = false;
        if (std::fclose(file) != 0)
            ok = false;
    }
    if (ok)
        ok = std::rename(temporary, destination.c_str()) == 0;
    if (!ok)
        std::remove(temporary);
    esp_vfs_fat_sdcard_unmount("/arcadesd", card);
    return ok ? std::string("Backup gravado em /pipoca5/") + filename + " (" +
                    std::to_string(count) + " registros)."
              : "Falha no backup microSD. Os dados internos foram preservados.";
}
} // namespace arcade
