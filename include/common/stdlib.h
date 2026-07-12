#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int32_t c, size_t n);
void bzero(void *dest, size_t n);
char *itoa(int32_t value, char *buf, int32_t base);

#endif
