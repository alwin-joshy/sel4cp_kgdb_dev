#include <microkit.h>

#define PINGPONG_CHANNEL 1

uintptr_t uart_base_vaddr;

#define UART_OFFSET 0x4c0
#define UART_WFIFO  0x0
#define UART_RFIFO  0x4
#define UART_CTRL 0x8
#define UART_STATUS 0xC

#define UART_TX_FULL        (1 << 21)
#define UART_RX_EMPTY       (1 << 20)
#define UART_CONTROL_TX_ENABLE   (1 << 12)

#define REG_PTR(base, offset) ((volatile uint32_t *)((base) + (offset)))

void uart_init() {
    *REG_PTR(uart_base_vaddr + UART_OFFSET, UART_CTRL) |= UART_CONTROL_TX_ENABLE;
}

void uart_put_char(int ch) {
    while ((*REG_PTR(uart_base_vaddr + UART_OFFSET, UART_STATUS) & UART_TX_FULL));

    /* Add character to the buffer. */
    *REG_PTR(uart_base_vaddr + UART_OFFSET, UART_WFIFO) = ch;
    if (ch == '\n') {
        uart_put_char('\r');
    }
}

void uart_put_str(char *str) {
    while (*str) {
        uart_put_char(*str);
        str++;
    }
}

void notified(microkit_channel ch) {
    switch (ch) {
    case PINGPONG_CHANNEL: {
        uart_put_str("Pong!\n");
        microkit_notify(PINGPONG_CHANNEL);
        break;
    }
    }
}

void init() {
	uart_init();
	uart_put_str("Hi! I'm PONG!\n");
    // microkit_notify(300);
    // arm_sys_null(seL4_SysDebugEnterKGDB);
}
