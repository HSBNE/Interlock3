#pragma once

#include <stddef.h>

// =============================================================================
// Types
// =============================================================================

typedef enum config_key {
    CFG_KEY_API_KEY,
    CFG_KEY_CONFIG_VERSION,
    CFG_KEY_DEVICE_NAME,
    CFG_KEY_DEVICE_TYPE,
    CFG_KEY_LED_COUNT,
    CFG_KEY_LED_TYPE,
    CFG_KEY_PORTAL_ADDRESS,
    CFG_KEY_RFID_READER_TYPE,
    CFG_KEY_SKELETON_CARD,
    CFG_KEY_WIFI_PSK,
    CFG_KEY_WIFI_SSID,
    CFG_KEY_N_KEYS  // Sentinel, must be last.
} config_key_t;

typedef enum config_err {
    CONFIG_OK,
    CONFIG_ERR_INVALID_ARG,          // Error for invalid arguments
    CONFIG_ERR_MISSING_KEY,          // Error when the queered key is not found in the config.
    CONFIG_ERR_BAD_CONFIG_FILE,      // The config file is malformed
    CONFIG_ERR_MISSING_VALUE,        // Error when the key was found in the config but its value was empty.
    CONFIG_ERR_MISSING_CONFIG_FILE,  // The config file does not exist.
    CONFIG_ERR_FILE_SYSTEM,          // Generic file system error when accessing the config.
    CONFIG_ERR_TRUNCATED,            // The value was too long, so it was truncated.
} config_err_t;


const char* config_err_to_str(config_err_t err);


config_err_t config_value_get(config_key_t key, char* out_value, size_t size);
