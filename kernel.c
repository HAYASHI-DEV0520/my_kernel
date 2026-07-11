#include <stddef.h>   // size_t
#include <stdint.h>   // uint32_t, int32_t など固定幅整数型

/*
 * MMIO（Memory-Mapped I/O）書き込み
 * ハードウェアレジスタはメモリアドレスとして見えているため、
 * 指定アドレスに直接値を書き込むことで操作する。
 * volatile: コンパイラの最適化による書き込み省略を防ぐ
 */
static inline void mmio_write(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t*)reg = data;
}

/* MMIO 読み込み: 指定アドレスから32ビット値を読み取る */
static inline uint32_t mmio_read(uint32_t reg)
{
    return *(volatile uint32_t*)reg;
}

/*
 * ソフトウェアディレイ: count 回のループで時間を消費する
 * インラインアセンブリを使う理由: -O2 最適化で空ループが削除されるのを防ぐため
 * subs: count - 1 を計算しフラグ更新
 * bne:  count が 0 になるまでループ
 * %=:   同じ関数が複数回展開されてもラベルが衝突しないよう一意な番号に置換される
 */
static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count)       // 出力: count と同じレジスタに結果を書き戻す
            : [count]"0"(count) // 入力: 出力と同じレジスタを使う
            : "cc");            // フラグレジスタを変更することをコンパイラに通知
}

/*
 * ハードウェアレジスタアドレス定義
 * BCM2836/2837 データシートに基づく
 * raspi2 & 3: ペリフェラルベース = 0x3F000000
 * raspi1:     ペリフェラルベース = 0x20000000
 */
enum
{
    // GPIO レジスタベースアドレス
    GPIO_BASE = 0x3F200000, // raspi2 & 3 用。raspi1 は 0x20200000

    GPPUD     = (GPIO_BASE + 0x94), // GPIO Pull-up/down 制御
    GPPUDCLK0 = (GPIO_BASE + 0x98), // GPIO Pull-up/down クロック (GPIO 0〜31)

    // PL011 UART0 レジスタベースアドレス
    UART0_BASE = 0x3F201000, // raspi2 & 3 用。raspi1 は 0x20201000

    UART0_DR     = (UART0_BASE + 0x00), // Data Register: 送受信データ
    UART0_RSRECR = (UART0_BASE + 0x04), // 受信ステータス / エラークリア
    UART0_FR     = (UART0_BASE + 0x18), // Flag Register: 送受信バッファ状態
    UART0_ILPR   = (UART0_BASE + 0x20), // IrDA 低消費電力カウンタ（未使用）
    UART0_IBRD   = (UART0_BASE + 0x24), // Integer Baud Rate Divisor（ボーレート整数部）
    UART0_FBRD   = (UART0_BASE + 0x28), // Fractional Baud Rate Divisor（ボーレート小数部）
    UART0_LCRH   = (UART0_BASE + 0x2C), // Line Control Register: データ長・パリティ・FIFO
    UART0_CR     = (UART0_BASE + 0x30), // Control Register: UART/TX/RX 有効化
    UART0_IFLS   = (UART0_BASE + 0x34), // Interrupt FIFO Level Select
    UART0_IMSC   = (UART0_BASE + 0x38), // Interrupt Mask Set/Clear
    UART0_RIS    = (UART0_BASE + 0x3C), // Raw Interrupt Status
    UART0_MIS    = (UART0_BASE + 0x40), // Masked Interrupt Status
    UART0_ICR    = (UART0_BASE + 0x44), // Interrupt Clear Register
    UART0_DMACR  = (UART0_BASE + 0x48), // DMA 制御
    UART0_ITCR   = (UART0_BASE + 0x80), // テスト制御（通常未使用）
    UART0_ITIP   = (UART0_BASE + 0x84), // テスト入力
    UART0_ITOP   = (UART0_BASE + 0x88), // テスト出力
    UART0_TDR    = (UART0_BASE + 0x8C), // テストデータ
};

/*
 * UART0 初期化
 * GPIO 14(TX) / 15(RX) のプルアップ/ダウンを無効化し、
 * ボーレート 115200bps、8ビット、FIFO ありで設定する
 */
void uart_init()
{
    // UART を一旦無効化（初期化中の誤動作防止）
    mmio_write(UART0_CR, 0x00000000);

    // GPIO 14, 15 のプルアップ/ダウン抵抗を無効化
    // BCM データシート指定の手順: 設定 → 待機 → クロック → 待機 → クリア
    mmio_write(GPPUD, 0x00000000);               // step1: 無効化を選択
    delay(150);                                  // step2: 150サイクル待機
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15)); // step3: GPIO14, 15 にクロック
    delay(150);                                  // step4: 150サイクル待機
    mmio_write(GPPUDCLK0, 0x00000000);           // step5: クロックをクリア

    // 全割り込みフラグをクリア（0x7FF = 下位11ビットすべて1）
    mmio_write(UART0_ICR, 0x7FF);

    // ボーレート設定: 115200 bps
    // 計算式: baud = UARTclk / (16 * (IBRD + FBRD/64))h
    //        115200 = 3000000 / (16 * (1 + 40/64))
    mmio_write(UART0_IBRD, 1);   // 整数部 = 1
    mmio_write(UART0_FBRD, 40);  // 小数部 = 40

    // データ長 8ビット (bit5,6=11)、FIFO 有効化 (bit4=1)
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    // 全割り込みをマスク（ポーリング方式で動作するため割り込み不要）
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    // UART 有効化 (bit0)、TX 有効化 (bit8)、RX 有効化 (bit9)
    mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

/*
 * 1文字送信
 * UART0_FR の bit5 (TXFF: 送信 FIFO 満杯) が下がるまでポーリング待機してから送信
 */
void uart_putc(unsigned char c)
{
    while ( mmio_read(UART0_FR) & (1 << 5) ) { }
    mmio_write(UART0_DR, c);
}

/*
 * 1文字受信
 * UART0_FR の bit4 (RXFE: 受信 FIFO 空) が下がるまでポーリング待機してから読み取る
 */
unsigned char uart_getc()
{
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}

/* 文字列送信: ヌル終端まで uart_putc を繰り返す */
void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putc((unsigned char)str[i]);
}

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