#ifndef OMAP4430_H_
#define OMAP4430_H_

#define OMAP4_L4_PER_BASE      0x48000000
#define OMAP4_UART3_BASE       (OMAP4_L4_PER_BASE + 0x20000)

// UART Registers
#define OMAP4_UART_THR         (OMAP4_UART3_BASE + 0x00)
#define OMAP4_UART_LSR         (OMAP4_UART3_BASE + 0x14)
#define OMAP4_LSR_THRE         0x20

#endif
