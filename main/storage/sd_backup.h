#pragma once
#include "journal.h"
#include <string>
namespace arcade {
// Explicit archive of the active store. Shares the persistent SD mount.
std::string backup_to_sd(Backend &storage, unsigned game_count);
} // namespace arcade
