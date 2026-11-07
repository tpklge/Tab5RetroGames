#pragma once
#include <string>
namespace arcade {
constexpr const char *SdDirectory = "/arcadesd/pipoca5";
// Mounted once for saves and backups. Failed initialization can be retried.
// Never formats the card; SDMMC slot 0 and LDO 4 are specific to Tab5.
bool mount_sd();
std::string sd_error();
}
