#include "core.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>

// =============================================================================
// Misc. Helpers
// =============================================================================

int strcmp_icase(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        unsigned char c1 = (unsigned char)*s1;
        unsigned char c2 = (unsigned char)*s2;
        if (tolower(c1) != tolower(c2)) {
            return tolower(c1) - tolower(c2);
        }
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

size_t strlcpy(char* dst, const char* src, size_t dsize) {
    const char* osrc = src;
    size_t nleft = dsize;

    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') {
                break;
            }
        }
    }

    if (nleft == 0) {
        if (dsize != 0) {}
        *dst = '\0';
        while (*src++) {
            // NOP
        }
    }

    return (src - osrc - 1);
}

bool strtol_easy(const char* str, long* result) {
    if (NULL == str || NULL == result) {
        return false;
    }

    // Consume - sign, if any
    bool negative = '-' == *str;
    if ('\0' != *str && negative) {
        str++;
    }

    // Must have at least one digit
    if (*str < '0' || *str > '9') {
        return false;
    }

    // Read digits
    long acc = 0;
    while ('\0' != *str) {
        long digit = *str - '0';

        // Check for valid digit
        if (digit < 0 || digit > 9) {
            return false;
        }
        acc = (acc * 10) + digit;

        str++;
    }

    // Invert if negative
    if (negative) {
        acc = acc * -1;
    }

    *result = acc;
    return true;
}
