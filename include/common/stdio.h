#ifndef COMMON_STDIO_H
#define COMMON_STDIO_H

#include <stdint.h>
#include <stddef.h>

int32_t getc(void);
char *gets(char *buf, size_t size);
void putc(char c);
void puts(const char *str);

#endif
