#ifndef COMMON_STDIO_H
#define COMMON_STDIO_H

#include <stdint.h>

int32_t getc(void);
char *gets(char *buf, int32_t size);
void putc(char c);
void puts(const char *str);

#endif
