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

void trap(const char* reason) {
    while (1) {
        ESP_LOGE(TAG, "Trapped. %s", reason);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    // Delay a bit at startup so I can attach my crappy programmer
    for (int i = 0; i < 3; i++) {
        ESP_LOGE(TAG, "Waiting (%d s)", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Start the file system
    const char* fs_status = "";
    if (!fs_init(&fs_status)) {
        trap(fs_status);
    }

    // Init the config
    if (!config_init()) {
        trap("Config not OK");
    }

    // Start the network
    network_start(config_get_wifi_ssid(), config_get_wifi_psk());

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP_LOGI(TAG, "OK");
    }
}