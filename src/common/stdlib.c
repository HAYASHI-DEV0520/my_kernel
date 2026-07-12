#include "common/stdlib.h"
#include "stdlib.h"

void *memcpy(void *dest, const void *src, size_t n)
{
    asm volatile(
        "   cmp     %[n], #0        \n"
        "   beq     2f              \n"
        "1: ldrb    r3, [%[src]], #1\n"
        "   strb    r3, [%[dst]], #1\n"
        "   subs    %[n], %[n], #1  \n"
        "   bne     1b              \n"
        "2:                         \n"
        : [dst] "+r"(dest), [src] "+r"(src), [n] "+r"(n)
        :
        : "r3", "cc", "memory"
    );
    return dest;
}

void *memset(void *dest, int32_t c, size_t n)
{
    asm volatile(
        "   cmp     %[n], #0        \n"
        "   beq     2f              \n"
        "   uxtb    r3, %[c]        \n"
        "1: strb    r3, [%[dst]], #1\n"
        "   subs    %[n], %[n], #1  \n"
        "   bne     1b              \n"
        "2:                         \n"
        : [dst] "+r"(dest), [n] "+r"(n)
        : [c] "r"(c)
        : "r3", "cc", "memory"
    );
    return dest;
}

void bzero(void *dest, size_t n)
{
    memset(dest, 0, n);
}

char *itoa(int32_t value, char *buf, int32_t base)
{
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char tmp[32];
    size_t i = 0;
    int32_t negative = 0;

    if (base < 2 || base > 36)
        return buf;

    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }

    uint32_t uval = (uint32_t)value;
    do {
        tmp[i++] = digits[uval % base];
        uval /= base;
    } while (uval);

    if (negative)
        tmp[i++] = '-';

    size_t j = 0;
    while (i > 0)
        buf[j++] = tmp[--i];
    buf[j] = '\0';

    return buf;
}