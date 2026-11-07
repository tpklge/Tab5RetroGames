#include "sd_backup.h"
#include "file_names.h"
#include "sd_card.h"
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
namespace arcade {
std::string backup_to_sd(Backend &storage, unsigned game_count) {
    if (!mount_sd())
        return sd_error() + ". Sem formatacao; insira o cartao e tente novamente.";
    const char *directory = SdDirectory;
    const char *temporary = "/arcadesd/pipoca5/backup.tmp";
    bool ok = mkdir(directory, 0755) == 0 || errno == EEXIST;
    std::string filename;
    std::string destination;
    // FAT rename does not replace existing targets. Select a free destination
    // and keep earlier backups intact, including across power interruptions.
    for (unsigned serial = 0; ok && serial < 10000; ++serial) {
        filename = backup_filename(serial);
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
                copy(rank_key(game, mode, difficulty));
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
    return ok ? std::string("Backup gravado em /pipoca5/") + filename + " (" +
                    std::to_string(count) + " registros)."
              : "Falha no backup microSD. Os registros de origem foram preservados.";
}
} // namespace arcade
