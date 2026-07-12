#include "common/stdio.h"
#include "kernel/uart.h"

int32_t getc(void) {
    return (int32_t)uart_getc();
}

char *gets(char *buf, size_t size) {
    size_t i = 0;
    if (size == 0) return buf;
    while (i < size - 1) {
        char c = (char)uart_getc();
        if (c == '\r' || c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return buf;
}

void putc(char c) {
    uart_putc((unsigned char)c);
}

void puts(const char *str) {
    uart_puts(str);
}



