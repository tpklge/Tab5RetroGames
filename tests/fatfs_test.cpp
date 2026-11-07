#include "file_names.h"
#include "ff.h"
#include "diskio.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
static std::array<BYTE, 8 * 1024 * 1024> disk{};
extern "C" {
PARTITION VolToPart[FF_VOLUMES] = {{0, 0}, {1, 0}};
DSTATUS disk_initialize(BYTE drive) { return drive == 0 ? 0 : STA_NOINIT; }
DSTATUS disk_status(BYTE drive) { return disk_initialize(drive); }
DRESULT disk_read(BYTE drive, BYTE *buffer, LBA_t sector, UINT count) {
    if (drive || (uint64_t(sector) + count) * 512 > disk.size()) return RES_PARERR;
    std::memcpy(buffer, disk.data() + size_t(sector) * 512, size_t(count) * 512); return RES_OK;
}
DRESULT disk_write(BYTE drive, const BYTE *buffer, LBA_t sector, UINT count) {
    if (drive || (uint64_t(sector) + count) * 512 > disk.size()) return RES_PARERR;
    std::memcpy(disk.data() + size_t(sector) * 512, buffer, size_t(count) * 512); return RES_OK;
}
DRESULT disk_ioctl(BYTE drive, BYTE command, void *buffer) {
    if (drive) return RES_PARERR;
    switch (command) {
        case CTRL_SYNC: case CTRL_TRIM: return RES_OK;
        case GET_SECTOR_COUNT: *static_cast<LBA_t *>(buffer) = disk.size() / 512; return RES_OK;
        case GET_SECTOR_SIZE: *static_cast<WORD *>(buffer) = 512; return RES_OK;
        case GET_BLOCK_SIZE: *static_cast<DWORD *>(buffer) = 1; return RES_OK;
        default: return RES_PARERR;
    }
}
DWORD get_fattime() { return (DWORD(2026 - 1980) << 25) | (11 << 21) | (6 << 16); }
int ff_mutex_create(int) { return 1; }
void ff_mutex_delete(int) {}
int ff_mutex_take(int) { return 1; }
void ff_mutex_give(int) {}
}
void roundtrip(const std::string &key) {
    const auto temporary = "0:/pipoca5/" + arcade::record_filename(key, true);
    const auto destination = "0:/pipoca5/" + arcade::record_filename(key);
    FIL file{}; UINT written = 0;
    assert(f_open(&file, temporary.c_str(), FA_CREATE_ALWAYS | FA_WRITE) == FR_OK);
    const char payload[] = "save payload";
    assert(f_write(&file, payload, sizeof(payload), &written) == FR_OK && written == sizeof(payload));
    assert(f_sync(&file) == FR_OK && f_close(&file) == FR_OK);
    auto removed = f_unlink(destination.c_str()); assert(removed == FR_OK || removed == FR_NO_FILE);
    assert(f_rename(temporary.c_str(), destination.c_str()) == FR_OK);
    assert(f_open(&file, destination.c_str(), FA_READ) == FR_OK);
    char buffer[sizeof(payload)]{}; UINT read = 0;
    assert(f_read(&file, buffer, sizeof(buffer), &read) == FR_OK && read == sizeof(buffer));
    assert(std::memcmp(buffer, payload, sizeof(buffer)) == 0 && f_close(&file) == FR_OK);
}
int main() {
    static_assert(FF_USE_LFN == 0, "Regression must use the firmware's short-name configuration");
    MKFS_PARM options{}; options.fmt = FM_FAT | FM_SFD;
    std::array<BYTE, 4096> work{};
    assert(f_mkfs("0:", &options, work.data(), work.size()) == FR_OK);
    FATFS filesystem{}; assert(f_mount(&filesystem, "0:", 1) == FR_OK);
    assert(f_mkdir("0:/pipoca5") == FR_OK);
    FIL file{};
    assert(f_open(&file, "0:/pipoca5/save0a.bin.tmp", FA_CREATE_ALWAYS | FA_WRITE) == FR_INVALID_NAME);
    assert(f_open(&file, "0:/pipoca5/backup-0001.bin", FA_CREATE_ALWAYS | FA_WRITE) == FR_INVALID_NAME);
    for (int game = 0; game < 11; ++game) for (char slot : {'a', 'b'}) {
        const auto key = "save" + std::to_string(game) + slot;
        roundtrip(key); roundtrip(key);
        for (int mode = 0; mode < 4; ++mode) for (int difficulty = 0; difficulty < 4; ++difficulty)
            roundtrip(arcade::rank_key(unsigned(game),unsigned(mode),unsigned(difficulty)) + slot);
    }
    roundtrip("configa"); roundtrip("configb");
    for (unsigned serial : {0u, 1u, 9999u}) {
        auto name = "0:/pipoca5/" + arcade::backup_filename(serial);
        assert(f_open(&file, name.c_str(), FA_CREATE_ALWAYS | FA_WRITE) == FR_OK);
        assert(f_close(&file) == FR_OK);
    }
    assert(arcade::rank_key(10,0,0)=="rankA00");
    assert(arcade::rank_key(9,3,3)=="rank933");
    assert(arcade::record_filename("123456789").empty());
    assert(arcade::backup_filename(10000).empty());
    assert(f_mount(nullptr, "0:", 0) == FR_OK);
    puts("PASS real FatFs: reproduced invalid old names; new temp/save/config/352 ranking slots/backup names write, sync, rename and read with LFN disabled");
}
