#pragma once
using esp_err_t = int;
constexpr esp_err_t ESP_OK = 0, ESP_FAIL = -1, ESP_ERR_NO_MEM = 0x101,
                    ESP_ERR_TIMEOUT = 0x107, ESP_ERR_INVALID_RESPONSE = 0x108,
                    ESP_ERR_INVALID_CRC = 0x109;
const char *esp_err_to_name(esp_err_t error);
