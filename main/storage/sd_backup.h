#pragma once
#include "journal.h"
#include <string>
namespace arcade {
// Explicit complementary backup. The NVS remains the primary store.
std::string backup_to_sd(Backend &storage, unsigned game_count);
} // namespace arcade
