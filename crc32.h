#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdio.h>
#include <stdint.h>
uint32_t crc32(uint32_t crc, const void *buf, size_t size);

#endif
