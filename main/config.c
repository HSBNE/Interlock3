#include "config.h"
#include <stddef.h>
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
    }
    return "INVALID";
}

// =============================================================================
// Keys
// =============================================================================

static const char* config_key_strings[CFG_KEY_N_KEYS] = {
    [CFG_KEY_API_KEY] = "API_KEY",
    [CFG_KEY_CONFIG_VERSION] = "CONFIG_VERSION",
    [CFG_KEY_DEVICE_NAME] = "DEVICE_NAME",
    [CFG_KEY_DEVICE_TYPE] = "DEVICE_TYPE",
    [CFG_KEY_LED_COUNT] = "LED_COUNT",
    [CFG_KEY_LED_TYPE] = "LED_TYPE",
    [CFG_KEY_PORTAL_ADDRESS] = "PORTAL_ADDRESS",
    [CFG_KEY_RFID_READER_TYPE] = "RFID_READER_TYPE",
    [CFG_KEY_SKELETON_CARD] = "SKELETON_CARD",
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
// be null terminated for any `size` greater than zero.
//
// On failure, a relevant error code will be returned. The value in `out_value`
// must be ignored.
config_err_t config_value_get(config_key_t key, char* out_value, size_t size) {
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
