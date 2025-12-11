#pragma once

#include <stddef.h>
#include "core.h"

// =============================================================================
// Types
// =============================================================================

typedef enum config_key {
    CFG_KEY_CONFIG_VERSION,
    CFG_KEY_DEVICE_NAME,
    CFG_KEY_DEVICE_TYPE,
    CFG_KEY_LED_COUNT,
    CFG_KEY_LED_TYPE,
    CFG_KEY_PORTAL_ADDRESS,
    CFG_KEY_PORTAL_API_KEY,
    CFG_KEY_PORTAL_PORT,
    CFG_KEY_RFID_READER_TYPE,
    CFG_KEY_RFID_SKELETON_CARD,
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
    CONFIG_ERR_INVALID_VALUE,        // A config item has an invalid value
    CONFIG_ERR_TRUNCATED,            // The value was too long, so it was truncated.
    CONFIG_ERR_N_ERRS                // Sentinel, must be last
} config_err_t;

const char* config_err_to_str(config_err_t err);

// =============================================================================
// Interface
// =============================================================================

// Initialise the config system.
//
// Must come after file system initialization
//
// This must be called, and succeed, before calling any of the config_get
// functions.
//
// Returns true on success, false otherwise.
bool config_init(void);

// Device
device_type_t config_get_device_type(void);
const char* config_get_device_name(void);

// Portal
const char* config_get_portal_address(void);
const char* config_get_portal_api_key(void);
uint16_t config_get_portal_port(void);

// WIFI
const char* config_get_wifi_ssid(void);
const char* config_get_wifi_psk(void);

// LED
uint16_t config_get_led_count(void);
led_type_t config_get_led_type(void);

// RFID
rfid_reader_type_t config_get_rfid_reader_type(void);
bool config_get_rfid_use_skeleton_card(void);
rfid_number_t config_get_skeleton_card(void);
