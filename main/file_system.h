#pragma once

#include "lfs.h"

#include "freertos/FreeRTOS.h"

// Initialises the file system, handles mounting, etc.
//
// If `out_status` is not NULL, a human readble status will be placed there.
//
// Returns true if the filesystem is OK to use, false on failure.
bool fs_init(const char** out_status);

// Obtain the file system and lock the mutex. When the caller is done with the
// file system it must return it using fs_unlock.
//
// Must only be called after fs_init()
//
// Will return NULL on failure (file system not available or mutex timed out).
lfs_t* fs_get_and_lock(TickType_t max_delay);

// Release the file system mutex. Does nothing if NULL is passed to `fs`.
void fs_unlock(lfs_t* fs);

// fgets style function designed to work with LittleFS.
// Supports LF/CRLF/CR line endings, but will always convert to LF endings (only 
// a single '\n' will appear at the end of the buffer).
// Always NULL terminates (except for size == 0) on success.
// On success `buffer` will be returned. On failure, NULL.
char* fs_fgets(char* buffer, size_t size, lfs_file_t* file, lfs_t* fs);