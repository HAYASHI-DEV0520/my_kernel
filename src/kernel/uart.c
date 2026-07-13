#include <stddef.h>
#include <stdint.h>

#include "kernel/uart.h"

static inline void mmio_write(uintptr_t reg, uint32_t data)
{
    *(volatile uint32_t *)reg = data;
}

static inline uint32_t mmio_read(uintptr_t reg)
{
    return *(volatile uint32_t *)reg;
}

/* BCM2711 (Raspberry Pi 4B) peripheral base: 0xFE000000 */
#define GPIO_BASE    ((uintptr_t)0xFE200000)
#define GPFSEL1      (GPIO_BASE + 0x04)
#define GPPUPPDN0    (GPIO_BASE + 0xE4)

#define UART0_BASE   ((uintptr_t)0xFE201000)
#define UART0_DR     (UART0_BASE + 0x00)
#define UART0_FR     (UART0_BASE + 0x18)
#define UART0_IBRD   (UART0_BASE + 0x24)
#define UART0_FBRD   (UART0_BASE + 0x28)
#define UART0_LCRH   (UART0_BASE + 0x2C)
#define UART0_CR     (UART0_BASE + 0x30)
#define UART0_IMSC   (UART0_BASE + 0x38)
#define UART0_ICR    (UART0_BASE + 0x44)

void uart_init(void)
{
    /* Disable UART */
    mmio_write(UART0_CR, 0x00000000);

    /* Set GPIO14 (TX) and GPIO15 (RX) to AF0 (UART0)
     * GPFSEL1 bits [14:12] = GPIO14, bits [17:15] = GPIO15, AF0 = 0b100 */
    uint32_t sel = mmio_read(GPFSEL1);
    sel &= ~((uint32_t)0x7 << 12);
    sel &= ~((uint32_t)0x7 << 15);
    sel |=  ((uint32_t)0x4 << 12);
    sel |=  ((uint32_t)0x4 << 15);
    mmio_write(GPFSEL1, sel);

    /* BCM2711: disable pull-up/down on GPIO14 and GPIO15 via GPPUPPDN0
     * 2 bits per pin: GPIO14 = bits [29:28], GPIO15 = bits [31:30], 0b00 = no pull */
    uint32_t pud = mmio_read(GPPUPPDN0);
    pud &= ~((uint32_t)0xF << 28);
    mmio_write(GPPUPPDN0, pud);

    /* Clear pending interrupts */
    mmio_write(UART0_ICR, 0x7FF);

    /* Baud rate: UART clock = 48 MHz, target = 115200 baud
     * IBRD = 48000000 / (16 * 115200) = 26
     * FBRD = round((48000000 / (16 * 115200) - 26) * 64) = 3 */
    mmio_write(UART0_IBRD, 26);
    mmio_write(UART0_FBRD, 3);

    /* 8-bit word, FIFOs enabled, no parity, 1 stop bit */
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    /* Mask all interrupts */
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    /* Enable UART, TX, RX */
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(unsigned char c)
{
    while (mmio_read(UART0_FR) & (1 << 5)) { }
    mmio_write(UART0_DR, c);
}

unsigned char uart_getc(void)
{
    while (mmio_read(UART0_FR) & (1 << 4)) { }
    return (unsigned char)mmio_read(UART0_DR);
}

void uart_puts(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
        uart_putc((unsigned char)str[i]);
}
