#pragma once
#include "input_state.h"
namespace arcade::keyboard {
bool init();
void drain(InputState &state, uint32_t now);
void cleanup();
bool available();
} // namespace arcade::keyboard
