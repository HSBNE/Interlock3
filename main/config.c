#include "config.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "esp_log.h"
#include "file_system.h"
#include "lfs.h"
#include "portmacro.h"

// File path for the config file
#define CONFIG_FILE_PATH "/config.txt"

// The maximum length for the value in a configuration key/value pair.
#define CONFIG_MAX_VALUE_LENGTH 127

#define CONFIG_MAX_KEY_LENGTH 63

// The maximum length of a line in the config
// - The key length
// - 1 char for the '='
// - The value length
// - CRLF
#define CONFIG_MAX_LINE_LENGTH (CONFIG_MAX_KEY_LENGTH + 1 + CONFIG_MAX_VALUE_LENGTH + 2)

#define TAG "config"

// =============================================================================
// Notes
// =============================================================================

// The configuration is stored as key-value pairs in SPIFFS in config.txt. Each
// pair is separated by a newline and an optional line feed character (LF/CRLF).

// =============================================================================
// Helpers
// =============================================================================

const char* config_err_to_str(config_err_t err) {
    switch (err) {
        case CONFIG_OK:
            return "CONFIG_OK";
        case CONFIG_ERR_INVALID_ARG:
            return "CONFIG_ERR_INVALID_ARG";
        case CONFIG_ERR_MISSING_KEY:
            return "CONFIG_ERR_MISSING_KEY";
        case CONFIG_ERR_BAD_CONFIG_FILE:
            return "CONFIG_ERR_BAD_CONFIG_FILE";
        case CONFIG_ERR_MISSING_VALUE:
            return "CONFIG_ERR_MISSING_VALUE";
        case CONFIG_ERR_MISSING_CONFIG_FILE:
            return "CONFIG_ERR_MISSING_CONFIG_FILE";
        case CONFIG_ERR_FILE_SYSTEM:
            return "CONFIG_ERR_FILE_SYSTEM";
        case CONFIG_ERR_TRUNCATED:
            return "CONFIG_ERR_TRUNCATED";
        case CONFIG_ERR_INVALID_VALUE:
            return "CONFIG_ERR_INVALID_VALUE";
        case CONFIG_ERR_N_ERRS:
            // Intentional fall through
            (void)0;
    }
    return "INVALID";
}

// =============================================================================
// Keys
// =============================================================================

static const char* config_key_strings[CFG_KEY_N_KEYS] = {
    [CFG_KEY_CONFIG_VERSION] = "CONFIG_VERSION",
    [CFG_KEY_DEVICE_NAME] = "DEVICE_NAME",
    [CFG_KEY_DEVICE_TYPE] = "DEVICE_TYPE",
    [CFG_KEY_LED_COUNT] = "LED_COUNT",
    [CFG_KEY_LED_TYPE] = "LED_TYPE",
    [CFG_KEY_PORTAL_ADDRESS] = "PORTAL_ADDRESS",
    [CFG_KEY_PORTAL_API_KEY] = "PORTAL_API_KEY",
    [CFG_KEY_PORTAL_PORT] = "PORTAL_PORT",
    [CFG_KEY_RFID_READER_TYPE] = "RFID_READER_TYPE",
    [CFG_KEY_RFID_SKELETON_CARD] = "RFID_SKELETON_CARD",
    [CFG_KEY_WIFI_PSK] = "WIFI_PSK",
    [CFG_KEY_WIFI_SSID] = "WIFI_SSID",
};

#define DBG()                     \
    do {                          \
        printf("%d\n", __LINE__); \
        fflush(stdout);           \
    } while (0)

// =============================================================================
// File Parsing
// =============================================================================

// Attempts to read the value of a given key from the config. The value is only
// valid if the function returns CONFIG_OK.
//
// On success, up to `size` bytes will be copied to `out_value`, which will
// be null terminated for any `size` greater than zero. Size is recommended to
// be at least CONFIG_MAX_VALUE_LENGTH + 1.
//
// On failure, a relevant error code will be returned. The value in `out_value`
// must be ignored.
static config_err_t config_value_get(config_key_t key, char* out_value, size_t size) {
    // Validate the key
    if (0 > key || key >= CFG_KEY_N_KEYS) {
        return CONFIG_ERR_INVALID_ARG;
    }

    // Obtain file system
    lfs_t* fs = fs_get_and_lock(portMAX_DELAY);
    if (NULL == fs) {
        return CONFIG_ERR_FILE_SYSTEM;
    }

    // Open the file
    lfs_file_t config_file = {0};
    if (0 > lfs_file_open(fs, &config_file, CONFIG_FILE_PATH, LFS_O_RDONLY)) {
        ESP_LOGE(TAG, "Failed to open the config file for reading. Does it exist?");
        fs_unlock(fs);
        return CONFIG_ERR_MISSING_CONFIG_FILE;
    }

    // Fetch the string for the key we cant to find
    const char* key_str = config_key_strings[key];

    // If this never gets updated then the key is missing
    config_err_t ret_val = CONFIG_ERR_MISSING_KEY;

    // Read config line by line
    char line[CONFIG_MAX_LINE_LENGTH + 1] = {0};

    while (NULL != fs_fgets(line, CONFIG_MAX_LINE_LENGTH, &config_file, fs)) {
        // Skip empty lines and comments
        if ('\0' == line[0] || '#' == line[0] || ';' == line[0] || '\n' == line[0]) {
            continue;
        }

        // Check for truncation
        size_t len = strlen(line);
        if (len == (CONFIG_MAX_LINE_LENGTH - 1) && line[len - 1] != '\n') {
            ret_val = CONFIG_ERR_TRUNCATED;
            break;
        }

        // Remove trailing linefeeds and carrige returns
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
            len--;
            line[len] = '\0';
        }

        // Find where the value starts, and replace the K/V delimiter ('=') with
        // a null terminator.
        char* value = strchr(line, '=');
        if (NULL == value) {
            ret_val = CONFIG_ERR_BAD_CONFIG_FILE;
            break;
        }
        *value = '\0';
        value++;

        // Exit loop if we've found the key we're looking for
        if (0 == strcmp_icase(key_str, line)) {
            // Copy value out and check for truncation / missing value
            const size_t key_length = strlcpy(out_value, value, size);
            if (key_length >= size) {
                ret_val = CONFIG_ERR_TRUNCATED;
            } else if (0 == key_length) {
                ret_val = CONFIG_ERR_MISSING_VALUE;
            } else {
                ret_val = CONFIG_OK;
            }
            break;
        }
    }

    // Close the file
    lfs_file_close(fs, &config_file);

    // Return the file system
    fs_unlock(fs);

    return ret_val;
}

// =============================================================================
// Config
// =============================================================================

typedef struct interlock_config {
    // Device
    device_type_t device_type;
    char device_name[CONFIG_MAX_VALUE_LENGTH + 1];

    // Portal
    char portal_address[CONFIG_MAX_VALUE_LENGTH + 1];
    char portal_api_key[CONFIG_MAX_VALUE_LENGTH + 1];
    uint16_t portal_port;

    // WIFI
    char wifi_ssid[CONFIG_MAX_VALUE_LENGTH + 1];
    char wifi_psk[CONFIG_MAX_VALUE_LENGTH + 1];

    // LED
    uint16_t led_count;
    led_type_t led_type;

    // RFID
    rfid_reader_type_t rfid_reader_type;
    bool rfid_use_skeleton_card;
    rfid_number_t skeleton_card;
} interlock_config_t;

static bool config_str_to_device_type(const char* str, device_type_t* type) {
    if (0 == strcmp_icase(str, "DOOR")) {
        *type = DEVICE_TYPE_DOOR;
        return true;
    }

    if (0 == strcmp_icase(str, "INTERLOCK")) {
        *type = DEVICE_TYPE_INTERLOCK;
        return true;
    }

    return false;
}

static bool config_str_to_led_type(const char* str, led_type_t* type) {
    if (0 == strcmp_icase(str, "RGBW")) {
        *type = LED_TYPE_RGBW;
        return true;
    }

    if (0 == strcmp_icase(str, "BGRW")) {
        *type = LED_TYPE_BGRW;
        return true;
    }

    return false;
}

static bool config_str_to_rfid_reader_type(const char* str, rfid_reader_type_t* type) {
    if (0 == strcmp_icase(str, "RF125PS")) {
        *type = RFID_READER_TYPE_RF125PS;
        return true;
    }

    if (0 == strcmp_icase(str, "LEGACY")) {
        *type = RFID_READER_TYPE_LEGACY;
        return true;
    }

    return false;
}

static bool config_str_to_u16(const char* str, uint16_t* val) {
    long l;
    if (!strtol_easy(str, &l)) {
        return false;
    }

    if (l < 0 || l > UINT16_MAX) {
        return false;
    }

    *val = (uint16_t)l;
    return true;
}

static void config_read_From_file_log_error(config_key_t key, config_err_t err) {
    ESP_LOGE(TAG, "Error reading config value for %s: %s", config_key_strings[key], config_err_to_str(err));
}

static void config_read_from_file_helper(config_key_t key, char* buffer, size_t buffer_size, uint32_t* status) {
    config_err_t err = config_value_get(key, buffer, buffer_size);
    if (CONFIG_OK != err) {
        config_read_From_file_log_error(key, err);
        *status |= (1 << err);
    }
}

// Returns true if the config was successfully read.
static bool config_read_from_file(interlock_config_t* config) {
    uint32_t status = 0;                             // Bit field of errors
    char buffer[CONFIG_MAX_VALUE_LENGTH + 1] = {0};  // Working buffer non string config items

    memset(config, 0, sizeof(interlock_config_t));

    // Device type
    config_read_from_file_helper(CFG_KEY_DEVICE_TYPE, buffer, sizeof(buffer), &status);
    if (!config_str_to_device_type(buffer, &config->device_type)) {
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // Device name
    config_read_from_file_helper(CFG_KEY_DEVICE_NAME, config->device_name, sizeof(config->device_name), &status);

    // Portal address
    config_read_from_file_helper(CFG_KEY_PORTAL_ADDRESS, config->portal_address, sizeof(config->portal_address),
                                 &status);

    // Portal API key
    config_read_from_file_helper(CFG_KEY_PORTAL_API_KEY, config->portal_api_key, sizeof(config->portal_api_key),
                                 &status);

    // Portal port
    config_read_from_file_helper(CFG_KEY_PORTAL_PORT, buffer, sizeof(buffer), &status);
    if (!config_str_to_u16(buffer, &config->portal_port)) {
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // Wifi SSID
    config_read_from_file_helper(CFG_KEY_WIFI_SSID, config->wifi_ssid, sizeof(config->wifi_ssid), &status);

    // Wifi PSK
    config_read_from_file_helper(CFG_KEY_WIFI_PSK, config->wifi_psk, sizeof(config->wifi_psk), &status);

    // LED count
    config_read_from_file_helper(CFG_KEY_LED_COUNT, buffer, sizeof(buffer), &status);
    if (!config_str_to_u16(buffer, &config->led_count)) {
        config_read_From_file_log_error(CFG_KEY_LED_COUNT, CONFIG_ERR_INVALID_ARG);
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // LED Type
    config_read_from_file_helper(CFG_KEY_LED_TYPE, buffer, sizeof(buffer), &status);
    if (!config_str_to_led_type(buffer, &config->led_type)) {
        config_read_From_file_log_error(CFG_KEY_LED_TYPE, CONFIG_ERR_INVALID_ARG);
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // RFID reader type
    config_read_from_file_helper(CFG_KEY_RFID_READER_TYPE, buffer, sizeof(buffer), &status);
    if (!config_str_to_rfid_reader_type(buffer, &config->rfid_reader_type)) {
        config_read_From_file_log_error(CFG_KEY_RFID_READER_TYPE, CONFIG_ERR_INVALID_ARG);
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // RFID skeleton card
    long skeleton_card;
    config_read_from_file_helper(CFG_KEY_RFID_SKELETON_CARD, buffer, sizeof(buffer), &status);
    if (0 == strcmp_icase(buffer, "NONE")) {
        config->rfid_use_skeleton_card = false;
        config->skeleton_card = UINT64_MAX;
    } else if (strtol_easy(buffer, &skeleton_card) && skeleton_card > 0) {
        config->rfid_use_skeleton_card = true;
        config->skeleton_card = skeleton_card;
    } else {
        config_read_From_file_log_error(CFG_KEY_RFID_SKELETON_CARD, CONFIG_ERR_INVALID_ARG);
        status |= CONFIG_ERR_INVALID_ARG;
    }

    // Print out any config errors:
    if (0 != status) {
        ESP_LOGE(TAG, "The following errors were encountered when reading the config file:");
        for (int i = 0; i < CONFIG_ERR_N_ERRS; i++) {
            if ((status & (1 << i)) && i != CONFIG_OK) {
                ESP_LOGE(TAG, "  - %s", config_err_to_str(i));
            }
        }
    }

    return 0 == status;
}

static interlock_config_t config = {0};

bool config_init() {
    return config_read_from_file(&config);
}

// =============================================================================
// Getters
// =============================================================================

device_type_t config_get_device_type(void) {
    return config.device_type;
}

const char* config_get_device_name(void) {
    return config.device_name;
}

const char* config_get_portal_address(void) {
    return config.portal_address;
}

const char* config_get_portal_api_key(void) {
    return config.portal_api_key;
}

uint16_t config_get_portal_port(void) {
    return config.portal_port;
}

const char* config_get_wifi_ssid(void) {
    return config.wifi_ssid;
}

const char* config_get_wifi_psk(void) {
    return config.wifi_psk;
}

uint16_t config_get_led_count(void) {
    return config.led_count;
}

led_type_t config_get_led_type(void) {
    return config.led_type;
}

rfid_reader_type_t config_get_rfid_reader_type(void) {
    return config.rfid_reader_type;
}

bool config_get_rfid_use_skeleton_card(void) {
    return config.rfid_use_skeleton_card;
}

rfid_number_t config_get_skeleton_card(void) {
    return config.skeleton_card;
}
