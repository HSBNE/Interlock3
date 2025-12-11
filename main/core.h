#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =============================================================================
// Core Types
// =============================================================================

typedef enum device_type {
    DEVICE_TYPE_DOOR,
    DEVICE_TYPE_INTERLOCK,
} device_type_t;

typedef enum led_type {
    LED_TYPE_RGBW,
    LED_TYPE_BGRW,
} led_type_t;

typedef enum rfid_reader_type {
    RFID_READER_TYPE_RF125PS,
    RFID_READER_TYPE_LEGACY,
} rfid_reader_type_t;

typedef uint64_t rfid_number_t;

// =============================================================================
// Misc. Helpers
// =============================================================================

// Case insensitive strcmp
int strcmp_icase(const char *s1, const char *s2);

// Copy string src to buffer `dst` of size `dsize`. At most `dsize-`1 chars will 
// be copied. Always NULL terminates (unless `dsize` == 0).
// Returns strlen(`src`). If retval >= `dsize` then truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t dsize);

// An easy to use strtol style function (what a concept!)
//
// Does not check for overflows.
// 
// We don't bother with any strange edge cases. str should be a non-empty null 
// terminated string comprised only of digits and, for negative numbers, a single 
// '-' sign at the start. Whitespace will cause a failure. Other characters will 
// cause a failure.
//
// If the function returns true you can use the result. If the function returns 
// false the the result is invalid and can not be used.
bool strtol_easy(const char* str, long *result);


