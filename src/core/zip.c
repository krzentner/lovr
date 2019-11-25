#include "zip.h"
#include <string.h>
#include <time.h>

static uint16_t readu16(const uint8_t* p) { uint16_t x; memcpy(&x, p, sizeof(x)); return x; }
static uint32_t readu32(const uint8_t* p) { uint32_t x; memcpy(&x, p, sizeof(x)); return x; }
static uint64_t readu64(const uint8_t* p) { uint64_t x; memcpy(&x, p, sizeof(x)); return x; }

bool zip_open(uint8_t* data, size_t size, size_t* cursor, uint64_t* count) {
  uint8_t* p = data + size - 22;
  if (size < 22 || readu32(p) != 0x06054b50) {
    return false;
  }

  // Zip64
  uint8_t* q = data + size - 42;
  if (size >= 42 && readu32(q) == 0x07064b50) {
    uint64_t offset = readu64(q + 8);
    q = data + offset;
    if (size >= offset + 56 && readu32(q) == 0x06064b50) {
      *count = readu64(q + 32);
      *cursor = readu64(q + 48);
      return true;
    }
  }

  *count = readu16(p + 10);
  *cursor = readu32(p + 16);
  return true;
}

bool zip_next(uint8_t* data, size_t size, size_t* cursor, zip_info* info) {
  uint8_t* p = data + *cursor;
  if (size < *cursor + 46 || readu32(p) != 0x02014b50) {
    return false;
  }

  uint16_t mtime = readu16(p + 12);
  uint16_t mdate = readu16(p + 14);
  struct tm t;
  memset(&t, 0, sizeof(t));
  t.tm_isdst = -1;
  t.tm_year = ((mdate >> 9) & 127) + 80;
  t.tm_mon = ((mdate >> 5) & 15) - 1;
  t.tm_mday = mdate & 31;
  t.tm_hour = (mtime >> 11) & 31;
  t.tm_min = (mtime >> 5) & 63;
  t.tm_sec = (mtime << 1) & 62;
  info->modtime = mktime(&t);

  info->size = readu32(p + 24);
  info->length = readu16(p + 28);
  info->offset = readu32(p + 42);
  info->name = (const char*) (p + 46);
  *cursor += 46 + info->length + readu16(p + 30) + readu16(p + 32);
  return *cursor < size;
}

void* zip_load(uint8_t* data, size_t size, size_t offset, size_t* csize, bool* compressed) {
  uint8_t* p = data + offset;
  if (offset + 30 >= size || readu32(p) != 0x04034b50) {
    return NULL;
  }

  uint16_t compression;
  *compressed = compression = readu16(p + 8);
  if (compression != 0 && compression != 8) {
    return false;
  }

  *csize = readu32(p + 18);
  uint32_t skip = readu16(p + 26) + readu16(p + 28);
  return offset + 30 + skip + *csize <= size ? (p + 30 + skip) : NULL;
}
