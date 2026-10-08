#include "audio_manager.h"
#include "m5tab5_driver_common.h"
#include "i2c_bus.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <atomic>
#include <algorithm>
#include <cmath>
namespace arcade::audio {
namespace {
i2s_chan_handle_t output;
QueueHandle_t events;
bool ready;
bool output_enabled;
TaskHandle_t task;
i2c_bus_device_handle_t codec, amp;
std::atomic<uint8_t> main_volume{65}, music_volume{35}, fx_volume{65};
std::atomic<bool> muted{false}, music_on{true};
std::atomic<float> motor{0};
struct Voice {
    float phase = 0, frequency = 0;
    int remaining = 0, total = 1;
};
float square(float &phase, float frequency) {
    phase += frequency / 16000.f;
    phase -= std::floor(phase);
    return phase < .5f ? 1.f : -1.f;
}
void synth(void *) {
    // Original sixteen-note loop, synthesized locally; no copyrighted recording.
    constexpr float tune[] = {261.63f, 329.63f, 392.f,   523.25f, 440.f,   392.f, 329.63f, 293.66f,
                              349.23f, 440.f,   523.25f, 659.25f, 587.33f, 440.f, 392.f,   293.66f};
    Voice voices[4];
    unsigned slot = 0;
    float melody_phase = 0, bass_phase = 0, motor_phase = 0;
    uint32_t song = 0;
    int16_t pcm[320];
    for (;;) {
        Effect effect;
        while (xQueueReceive(events, &effect, 0) == pdTRUE) {
            unsigned id = unsigned(effect);
            Voice &voice = voices[slot++ % 4];
            voice.phase = 0;
            voice.frequency = 180.f + id * 85.f;
            voice.total = voice.remaining = (effect == Effect::GameOver ? 9600
                                             : effect == Effect::Line   ? 4800
                                                                        : 1600);
        }
        for (unsigned i = 0; i < 160; ++i) {
            float mix = 0;
            for (auto &voice : voices)
                if (voice.remaining > 0) {
                    float decay = float(voice.remaining--) / voice.total;
                    mix += square(voice.phase, voice.frequency * (.65f + .35f * decay)) * .12f *
                           decay * fx_volume.load() / 100.f;
                }
            if (music_on.load()) {
                unsigned note = (song / 4000) % 16;
                mix += (square(melody_phase, tune[note]) * .12f +
                        square(bass_phase, tune[(note / 4) * 4] / 2) * .06f) *
                       music_volume.load() / 100.f;
                ++song;
            }
            float speed = motor.load();
            if (speed > 0)
                mix += square(motor_phase, 45.f + speed * .4f) * .11f * fx_volume.load() / 100.f;
            float amplitude = muted.load() ? 0.f : mix * main_volume.load() / 100.f;
            int16_t sample = int16_t(std::clamp(amplitude, -.8f, .8f) * 32767);
            pcm[2 * i] = pcm[2 * i + 1] = sample;
        }
        size_t written = 0;
        if (i2s_channel_write(output, pcm, sizeof(pcm), &written, 100) != ESP_OK ||
            written != sizeof(pcm)) {
            ESP_LOGW("audio", "PCM output interrupted");
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}
} // namespace
bool init() {
    if (ready)
        return true;
    auto fail = [] {
        cleanup();
        return false;
    };
    auto bus = m5::tab5::m5tab5_get_sys_i2c_bus();
    if (!bus)
        return false;
    codec = i2c_bus_device_create(bus, 0x10, 100000);
    amp = i2c_bus_device_create(bus, 0x43, 400000);
    if (!codec || !amp)
        return fail();
    // ES8388 DAC slave, 16-bit I2S stereo, MCLK/LRCK=256; ADC remains off.
    const uint8_t registers[][2] = {
        {0, 0x80},    {0, 0x12},    {0x19, 0x04}, {1, 0x50},    {2, 0},       {3, 0xff},
        {4, 0xc0},    {0x35, 0xa0}, {0x37, 0xd0}, {0x39, 0xd0}, {8, 0},       {0x17, 0x18},
        {0x18, 2},    {0x1a, 0},    {0x1b, 0},    {0x26, 0},    {0x27, 0x90}, {0x2a, 0x90},
        {0x2b, 0x80}, {0x2d, 0},    {0x2e, 0x1e}, {0x2f, 0x1e}, {0x30, 0x1e}, {0x31, 0x1e},
        {2, 0xf0},    {2, 0},       {4, 0x3c},    {0x19, 0}};
    for (const auto &reg : registers) {
        if (i2c_bus_write_byte(codec, reg[0], reg[1]) != ESP_OK)
            return fail();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    uint8_t reg = 0;
    if (i2c_bus_read_byte(amp, 5, &reg) != ESP_OK)
        return fail();
    if (i2c_bus_write_byte(amp, 5, reg | 2) != ESP_OK)
        return fail();
    i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    channel.dma_desc_num = 4;
    channel.dma_frame_num = 160;
    channel.auto_clear = true;
    if (i2s_new_channel(&channel, &output, nullptr) != ESP_OK)
        return fail();
    i2s_std_config_t config{};
    config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000);
    config.slot_cfg =
        I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    config.gpio_cfg.mclk = GPIO_NUM_30;
    config.gpio_cfg.bclk = GPIO_NUM_27;
    config.gpio_cfg.ws = GPIO_NUM_29;
    config.gpio_cfg.dout = GPIO_NUM_26;
    config.gpio_cfg.din = I2S_GPIO_UNUSED;
    if (i2s_channel_init_std_mode(output, &config) != ESP_OK ||
        i2s_channel_enable(output) != ESP_OK)
        return fail();
    output_enabled = true;
    events = xQueueCreate(32, sizeof(Effect));
    if (!events)
        return fail();
    ready = xTaskCreatePinnedToCore(synth, "arcade_audio", 4096, nullptr, 4, &task, 0) == pdPASS;
    if (!ready)
        return fail();
    return ready;
}
void cleanup() {
    ready = false;
    if (task) {
        vTaskDelete(task);
        task = nullptr;
    }
    if (output) {
        if (output_enabled)
            i2s_channel_disable(output);
        i2s_del_channel(output);
        output = nullptr;
        output_enabled = false;
    }
    if (events) {
        vQueueDelete(events);
        events = nullptr;
    }
    if (amp) {
        uint8_t value = 0;
        if (i2c_bus_read_byte(amp, 5, &value) == ESP_OK)
            i2c_bus_write_byte(amp, 5, value & ~2u);
        i2c_bus_device_delete(&amp);
    }
    if (codec) {
        i2c_bus_write_byte(codec, 0x19, 0x04);
        i2c_bus_device_delete(&codec);
    }
}
void play(Effect effect) {
    if (ready)
        xQueueSend(events, &effect, 0);
}
void configure(uint8_t master, uint8_t music, uint8_t effects, bool mute) {
    main_volume = std::min(uint8_t(100), master);
    music_volume = std::min(uint8_t(100), music);
    fx_volume = std::min(uint8_t(100), effects);
    muted = mute;
}
void menu_music(bool enabled) { music_on = enabled; }
void engine(float speed) { motor = std::clamp(speed, 0.f, 300.f); }
bool available() { return ready; }
} // namespace arcade::audio
