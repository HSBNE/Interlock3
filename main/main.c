#include <stdio.h>
#include <string.h>
#include "config.h"
#include "core.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_spiffs.h"
#include "projdefs.h"

#define TAG "interlock"

// Init the file system
// Returns true if the file system is OK, false otherwise.
static bool init_file_system(void) {
    const esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path = CORE_SPIFFS_MOUNT,  //
        .partition_label = NULL,         //
        .max_files = 3,                  //
        .format_if_mount_failed = false  //
    };

    esp_err_t err = esp_vfs_spiffs_register(&spiffs_config);
    if (ESP_FAIL == err) {
        ESP_LOGE(TAG, "Failed to mount or format file system");
    } else if (ESP_ERR_NOT_FOUND == err) {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS. Is the partition table correct? Error: %s", esp_err_to_name(err));
    }
    return ESP_OK == err;
}

void app_main(void) {
    for (int i = 0; i < 5; i++) {
        ESP_LOGE(TAG, "Waiting (%d s)", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    const bool file_system_ok = init_file_system();

    size_t total = 0;
    size_t used = 0;
    if (ESP_OK == esp_spiffs_info(NULL, &total, &used)) {
        ESP_LOGE(TAG, "SPIFFS %d of %d bytes used.", (int)used, (int)total);
        ;
    } else {
        ESP_LOGE(TAG, "Could not obtain SPIFFS usage");
    }

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(3000));

        if (!file_system_ok) {
            ESP_LOGE(TAG, "File system not OK");
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