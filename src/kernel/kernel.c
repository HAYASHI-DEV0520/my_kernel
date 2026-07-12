#include <stddef.h>   // size_t
#include <stdint.h>   // uint32_t, int32_t など固定幅整数型

#include "kernel/uart.h"

/*
 * カーネルエントリポイント（boot.S の blx r3 から呼ばれる）
 * r0:    ブートローダ渡し値（0固定）
 * r1:    ARM マシン種別 ID
 * atags: ブートローダからのハードウェア情報ポインタ（ATAGS またはデバイスツリー）
 */
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    // 未使用引数の警告抑制（-Wextra 対策）
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    uart_puts("Hello, kernel World!\r\n");

    uart_puts("input:");

    // 受信した文字をそのままエコーバックし続ける
    while (1) {
        uart_putc(uart_getc());
        uart_puts("\ninput:");
    }
}
