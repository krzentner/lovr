#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Status:
//  - Lil'endian only
//  - Zip64 is not supported
//  - Only supports store and deflate compression
//  - No comment allowed at the end of archive (file comments are okay)
//  - No multi-disk archives
//  - No encryption

#pragma once

typedef struct {
  uint64_t offset;
  uint64_t size;
  uint64_t modtime;
  const char* name;
  uint16_t length;
} zip_info;

bool zip_open(uint8_t* data, size_t size, size_t* cursor, uint64_t* count);
bool zip_next(uint8_t* data, size_t size, size_t* cursor, zip_info* info);
void* zip_load(uint8_t* data, size_t size, size_t offset, size_t* csize, bool* compressed);
