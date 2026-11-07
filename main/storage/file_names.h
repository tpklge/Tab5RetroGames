#pragma once
#include <cstdio>
#include <string>
namespace arcade {
// The firmware uses FatFs without LFN: every component must fit FAT 8.3.
inline std::string record_filename(const std::string &key, bool temporary = false) {
    if (key.empty() || key.size() > 8) return {};
    for (char c : key)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-')) return {};
    return key + (temporary ? ".tmp" : ".bin");
}
inline std::string rank_key(unsigned game, unsigned mode, unsigned difficulty) {
    constexpr char ids[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (game >= 36 || mode >= 4 || difficulty >= 4) return {};
    return std::string("rank") + ids[game] + char('0' + mode) + char('0' + difficulty);
}
inline std::string backup_filename(unsigned serial) {
    if (!serial) return "backup.bin";
    if (serial > 9999) return {};
    char name[13];
    std::snprintf(name, sizeof(name), "bak%04u.bin", serial);
    return name;
}
}
