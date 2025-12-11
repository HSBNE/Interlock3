#include "file_system.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lfs.h"
#include "projdefs.h"
#include "spi_flash.h"

// =============================================================================
// Mutex
// =============================================================================

static SemaphoreHandle_t fs_mutex = {0};

// =============================================================================
// LittleFS Config and Buffers
// =============================================================================

// Partition
#define FS_PARTITION_OFFSET 0x12000
#define FS_PARTITION_SIZE 0x20000

// Block
#define FS_BLOCK_SIZE 4096  // Must be equal to flash sector size
#define FS_BLOCK_COUNT (FS_PARTITION_SIZE / FS_BLOCK_SIZE)
#define FS_FIRST_BLOCK (FS_PARTITION_OFFSET / FS_BLOCK_SIZE)
#define FS_BLOCK_CYCLES 500

// Read write
#define FS_READ_SIZE 256
#define FS_PROG_SIZE 256

// Buffers (32 bit aligned)
#define FS_CACHE_SIZE 256
#define FS_LOOKAHEAD_SIZE 256

static uint32_t fs_read_buffer[FS_CACHE_SIZE / sizeof(uint32_t)] = {0};
static uint32_t fs_prog_buffer[FS_CACHE_SIZE / sizeof(uint32_t)] = {0};
static uint32_t fs_lookahead_buffer[FS_LOOKAHEAD_SIZE / sizeof(uint32_t)] = {0};

// =============================================================================
// LittleFS HAL
// =============================================================================

static int fs_hal_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
    const size_t addr = FS_PARTITION_OFFSET + (block * FS_BLOCK_SIZE) + off;
    return spi_flash_read(addr, buffer, size) == ESP_OK ? 0 : -1;
}

static int fs_hal_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer,
                       lfs_size_t size) {
    const size_t addr = FS_PARTITION_OFFSET + (block * FS_BLOCK_SIZE) + off;
    return spi_flash_write(addr, buffer, size) == ESP_OK ? 0 : -1;
}

static int fs_hal_erase(const struct lfs_config* c, lfs_block_t block) {
    const size_t sector = FS_FIRST_BLOCK + block;
    return spi_flash_erase_sector(sector) == ESP_OK ? 0 : -1;
}

static int fs_hal_sync(const struct lfs_config* c) {
    // Nothing to do
    return 0;
}

// =============================================================================
// Public Interface
// =============================================================================

static bool filesystem_mounted = false;
static lfs_t filesystem = {0};

bool fs_init(const char** out_status) {
    // Create mutex
    fs_mutex = xSemaphoreCreateMutex();

    // Configure LittleFS
    // LittleFS does not take a copy of this config, so it must have a static
    // lifetime.
    static const struct lfs_config config = {
        // Block device operations
        .read = fs_hal_read,    //
        .prog = fs_hal_prog,    //
        .erase = fs_hal_erase,  //
        .sync = fs_hal_sync,    //

        // Block device configuration
        .read_size = FS_READ_SIZE,            //
        .prog_size = FS_PROG_SIZE,            //
        .block_size = FS_BLOCK_SIZE,          //
        .block_count = FS_BLOCK_COUNT,        //
        .cache_size = FS_CACHE_SIZE,          //
        .lookahead_size = FS_LOOKAHEAD_SIZE,  //
        .block_cycles = FS_BLOCK_CYCLES,      //

        // Buffers
        .read_buffer = fs_read_buffer,            //
        .prog_buffer = fs_prog_buffer,            //
        .lookahead_buffer = fs_lookahead_buffer,  //
    };

    // Mount files
    int err = lfs_mount(&filesystem, &config);

    // We don't want to reformat, the file system should be available even the
    // first time the device is booted.
    if (err) {
        if (NULL != out_status) {
            *out_status = "Unable to mount file system. Have you flashed the config?";
        }
        return false;
    }

    if (NULL != out_status) {
        *out_status = "Filesystem OK";
    }
    filesystem_mounted = true;
    return true;
}

lfs_t* fs_get_and_lock(TickType_t max_delay) {
    lfs_t* fs = NULL;
    if (pdPASS == xSemaphoreTake(fs_mutex, max_delay)) {
        fs = &filesystem;
        if (!filesystem_mounted) {
            fs = NULL;
            xSemaphoreGive(fs_mutex);
        }
    }
    return fs;
}

void fs_unlock(lfs_t* fs) {
    if (NULL != fs) {
        xSemaphoreGive(fs_mutex);
    }
}

char* fs_fgets(char* buffer, size_t size, lfs_file_t* file, lfs_t* fs) {
    if (0 == size) {
        return NULL;
    }

    size_t index = 0;
    while (index < size - 1) {
        // Read in char
        uint8_t c;
        int res = lfs_file_read(fs, file, &c, 1);
        if (0 == res) {
            // EOF
            break;
        }
        if (0 > res) {
            // Error reading file
            return NULL;
        }

        // Handle CR and CRLF endings
        if ('\r' == c) {
            // Peek for CRLF
            uint8_t next_char;
            res = lfs_file_read(fs, file, &next_char, 1);
            if (0 > res) {
                // Error reading file
                return NULL;
            }

            // If it's not a CRLF roll it back
            if (1 == res) {
                if ('\n' != next_char) {
                    res = lfs_file_seek(fs, file, -1, LFS_SEEK_CUR);
                    if (0 > res) {
                        // File seek error
                        return NULL;
                    }
                }
            }

            // Convert to LF ending
            buffer[index++] = '\n';
            break;
        }

        // Copy to buffer
        buffer[index++] = c;

        // Check for LF ending
        if ('\n' == c) {
            break;
        }
    }

    // If no data was read return NULL
    if (0 == index) {
        return NULL;
    }

    // Null terminate
    buffer[index] = '\0';
    return buffer;
}
