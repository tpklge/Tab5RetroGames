#pragma once
#include "input_state.h"
namespace arcade::keyboard {
inline void drain(InputState &, uint32_t) {}
inline bool available() { return true; }
} // namespace arcade::keyboard
