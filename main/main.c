#include <stdio.h>
#include <string.h>
#include "config.h"
#include "core.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "file_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_spiffs.h"
#include "lib/littlefs/lfs.h"
#include "network.h"
#include "projdefs.h"

#define TAG "interlock"

void app_main(void) {
    for (int i = 0; i < 5; i++) {
        ESP_LOGE(TAG, "Waiting (%d s)", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    const char* fs_status = "";
    const bool file_system_ok = fs_init(&fs_status);
    ESP_LOGE(TAG, "File system status: %s", fs_status);

    network_start("ssid", "pass");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(3000));

        if (!file_system_ok) {
            ESP_LOGE(TAG, "File system status: %s", fs_status);
            continue;
        }

        for (int i = 0; i < CFG_KEY_N_KEYS; i++) {
            char buf[256] = {0};
            strcpy(buf, "didn't work :(");
            config_err_t err = config_value_get(i, buf, sizeof(buf));
            if (CONFIG_OK != err) {
                ESP_LOGE(TAG, "Error getting key %d: %s", i, config_err_to_str(err));
            } else {
                ESP_LOGI(TAG, "Value of key %d: %s", i, buf);
            }
        }
    }
}