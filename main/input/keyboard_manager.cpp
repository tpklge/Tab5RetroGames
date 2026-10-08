#include "keyboard_manager.h"
#include "m5_tab5_keyboard.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <atomic>
namespace arcade::keyboard {
namespace {
m5::M5Tab5Keyboard device;
QueueHandle_t queue;
std::atomic<bool> overflow{false};
bool ready;
bool begun;
void receive(m5_tab5_key_event_t event, void *) {
    if (event.type != M5_TAB5_KB_MODE_NORMAL)
        return;
    if (xQueueSend(queue, &event, 0) != pdTRUE)
        overflow = true;
}
} // namespace
bool init() {
    if (ready)
        return true;
    queue = xQueueCreate(128, sizeof(m5_tab5_key_event_t));
    if (!queue)
        return false;
    device.setLogLevel(M5_TAB5_KB_LOG_LEVEL_WARN);
    auto err = device.begin(I2C_NUM_1, 0x6d, 0, 1, 400000, M5_TAB5_KB_INT_MODE_POLLING);
    if (err != M5_TAB5_KB_OK) {
        ESP_LOGW("keyboard", "Keyboard unavailable: %d", int(err));
        vQueueDelete(queue);
        queue = nullptr;
        return false;
    }
    begun = true;
    ready = device.enableNormalMode(receive) == M5_TAB5_KB_OK &&
            device.setInterruptConfig(0x01) == M5_TAB5_KB_OK &&
            device.clearEventQueue() == M5_TAB5_KB_OK &&
            device.clearInterruptStatus() == M5_TAB5_KB_OK &&
            device.setInterruptMode(M5_TAB5_KB_INT_MODE_POLLING, 8) == M5_TAB5_KB_OK;
    if (!ready)
        cleanup();
    return ready;
}
void drain(InputState &state, uint32_t now) {
    if (!ready)
        return;
    if (overflow.exchange(false)) {
        xQueueReset(queue);
        state.clear();
        device.clearEventQueue();
        ESP_LOGW("keyboard", "Queue overflow: input released to avoid stuck controls");
    }
    m5_tab5_key_event_t event;
    for (unsigned count = 0; count < 128 && xQueueReceive(queue, &event, 0) == pdTRUE; ++count)
        state.event(matrix_key(event.row, event.col), event.pressed, now);
}
void cleanup() {
    if (begun) {
        device.setInterruptMode(M5_TAB5_KB_INT_MODE_DISABLED);
        device.enableNormalMode();
        device.setInterruptConfig(0x01);
        device.clearEventQueue();
        device.clearInterruptStatus();
        device.end();
        begun = false;
    }
    if (queue) {
        vQueueDelete(queue);
        queue = nullptr;
    }
    ready = false;
    overflow = false;
}
bool available() { return ready; }
} // namespace arcade::keyboard
