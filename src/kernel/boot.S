.section ".text.boot"

.global _start

_start:
    // Core 0 only — all other cores halt
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbnz    x1, halt

    // Set up stack (grows down from load address 0x80000)
    ldr     x1, =_start
    mov     sp, x1

    // Zero out BSS
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
    mov     x3, #0
.bss_loop:
    cmp     x1, x2
    b.ge    .bss_done
    str     x3, [x1], #8
    b       .bss_loop
.bss_done:

    bl      kernel_main

halt:
    wfe
    b       halt