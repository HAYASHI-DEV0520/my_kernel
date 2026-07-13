#include "kernel/uart.h"
#include "common/stdio.h"

/*
 * カーネルエントリポイント（boot.S の bl kernel_main から呼ばれる）
 * RPi 4B (AArch64): ファームウェアは x0 に DTB ポインタを渡すが現時点では未使用
 */
void kernel_main(void)
{
    uart_init();
    puts("Hello, kernel World!\r\n");

    puts("input:");

    // 受信した文字をそのままエコーバックし続ける
    while (1) {
        putc(uart_getc());
        puts("\r\ninput:");
    }
}
