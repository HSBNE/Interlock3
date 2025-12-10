#include "network.h"
#include <stdint.h>

#include "core.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "projdefs.h"
#include "tcpip_adapter.h"

// =============================================================================
// WiFi
// =============================================================================

// Event group for wifi connection
static EventGroupHandle_t wifi_event_group;

#define WIFI_EVENT_GROUP_CONNECTED_BIT (1 << 0)

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    // Wifi Events
    if (WIFI_EVENT == event_base) {

        // Connect to WiFi on start event
        if (WIFI_EVENT_STA_START == event_id) {
            esp_wifi_connect();
        }

        // Clear connected bit if we become disconnected
        if (WIFI_EVENT_STA_DISCONNECTED == event_id) {
            xEventGroupClearBits(wifi_event_group, WIFI_EVENT_GROUP_CONNECTED_BIT);
        }
    }

    // IP events
    if (IP_EVENT == event_base) {

        // Set connected bit if we got an IP
        if (IP_EVENT_STA_GOT_IP == event_id) {
            xEventGroupSetBits(wifi_event_group, WIFI_EVENT_GROUP_CONNECTED_BIT);
        }
    }
}

// This task is responsible for restarting the WiFi if it disconnects.
static void wifi_watchdog_task(void* ptr_to_wifi_task_ctx) {
    // Random delay to stop all the interlocks hammering the AP at the same time
    const TickType_t random_delay = pdMS_TO_TICKS(esp_random() % 1500);

    while (1) {
        if (!(xEventGroupGetBits(wifi_event_group) & WIFI_EVENT_GROUP_CONNECTED_BIT)) {
            ESP_LOGI("wifi_watchdog", "Attempting to reconnect to WiFi");
            esp_wifi_connect();
        }
        vTaskDelay(pdMS_TO_TICKS(5000) + random_delay);
    }
}

static void wifi_start(const char* wifi_ssid, const char* wifi_psk) {
    // Set up event group
    wifi_event_group = xEventGroupCreate();

    // Init Wifi
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    // Event handler
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Configure WiFi
    wifi_config_t wifi_config = {0};
    strlcpy((char*)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, wifi_psk, sizeof(wifi_config.sta.password));

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Start watchdog
    xTaskCreate(wifi_watchdog_task, "WiFi Watchdog", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}

// =============================================================================
// Network
// =============================================================================

void network_start(const char* wifi_ssid, const char* wifi_psk) {
    // Net
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Wifi
    wifi_start(wifi_ssid, wifi_psk);
}