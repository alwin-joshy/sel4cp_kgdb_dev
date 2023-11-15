#include <sel4cp.h>

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
    if (ch == '\r') {
        uart_put_char('\n');
    }
}

void uart_put_str(char *str) {
    while (*str) {
        uart_put_char(*str);
        str++;
    }
}


void init() {
	uart_init();
	uart_put_str("HELLO WORLD!\n");
    arm_sys_null(seL4_SysDebugEnterKGDB);
}

void notified(sel4cp_channel channel) {}
