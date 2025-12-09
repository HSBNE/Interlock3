#pragma once

#include <stddef.h>

// =============================================================================
// File System Configuration
// =============================================================================

#define CORE_SPIFFS_MOUNT "/spiffs"


// =============================================================================
// Core Types
// =============================================================================

typedef enum device_type {
    DEVICE_TYPE_DOOR,
    DEVICE_TYPE_INTERLOCK,
} device_type_t;

// =============================================================================
// Misc. Helpers
// =============================================================================

// Case insensitive strcmp
int strcmp_icase(const char *s1, const char *s2);

// Copy string src to buffer `dst` of size `dsize`. At most `dsize-`1 chars will 
// be copied. Always NULL terminates (unless `dsize` == 0).
// Returns strlen(`src`). If retval >= `dsize` then truncation occurred.
size_t strlcpy(char *dst, const char *src, size_t dsize);
