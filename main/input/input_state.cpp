#include "input_state.h"
#include <cstring>
namespace arcade {
uint8_t matrix_key(uint8_t row, uint8_t col) {
    // Official Normal-mode matrix, not a USB device. HID values are internal IDs.
    static constexpr uint8_t table[5][14] = {
        {0x29, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2d, 0x2e, 0x4c},
        {0x35, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2f, 0x30, 0x31},
        {0x2b, 0x14, 0x1a, 0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, 0x12, 0x13, 0x33, 0x34, 0x2a},
        {0xe3, 0xe1, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x52, 0x2d, 0x28},
        {0xe0, 0xe2, 0x1d, 0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x37, 0x50, 0x51, 0x4f, 0x2c}};
    return row < 5 && col < 14 ? table[row][col] : 0;
}
char key_character(uint8_t code, bool shift) {
    if (code >= 4 && code <= 29)
        return char((shift ? 'A' : 'a') + code - 4);
    if (code >= 0x1e && code <= 0x26)
        return char('1' + code - 0x1e);
    if (code == 0x27)
        return '0';
    if (code == key::Space)
        return ' ';
    if (code == 0x2d)
        return '-';
    if (code == 0x37)
        return '.';
    return 0;
}
void InputState::event(uint8_t code, bool pressed, uint32_t now) {
    if (!code || down[code] == pressed)
        return; // Reject duplicate firmware events.
    down[code] = pressed;
    if (pressed) {
        edges[code] = 1;
        started[code] = last_repeat[code] = now;
        repeating[code] = 0;
    } else {
        ups[code] = 1;
        repeating[code] = 0;
    }
}
bool InputState::repeat(uint8_t code, uint32_t now, uint32_t delay, uint32_t period) {
    if (edges[code])
        return true;
    if (!down[code] || uint32_t(now - started[code]) < delay)
        return false;
    if (!repeating[code] || uint32_t(now - last_repeat[code]) >= period) {
        repeating[code] = 1;
        last_repeat[code] = now;
        return true;
    }
    return false;
}
void InputState::end_frame() {
    edges.fill(0);
    ups.fill(0);
}
void InputState::clear() {
    down.fill(0);
    edges.fill(0);
    ups.fill(0);
    repeating.fill(0);
}
} // namespace arcade
