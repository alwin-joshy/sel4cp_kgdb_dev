#include <sel4cp.h>

uintptr_t uart_base_vaddr;

#define RHR_MASK 0b111111111
#define UARTDR 0x000
#define UARTFR 0x018
#define UARTIMSC 0x038
#define UARTICR 0x044
#define PL011_UARTFR_TXFF (1 << 5)
#define PL011_UARTFR_RXFE (1 << 4)

#define REG_PTR(base, offset) ((volatile uint32_t *)((base) + (offset)))

void uart_init() {
    *REG_PTR(uart_base_vaddr, UARTIMSC) = 0x50;
}


int uart_get_char() {
    int ch = 0;

    if ((*REG_PTR(uart_base_vaddr, UARTFR) & PL011_UARTFR_RXFE) == 0) {
        ch = *REG_PTR(uart_base_vaddr, UARTDR) & RHR_MASK;
    }

    return ch;
}

void uart_put_char(int ch) {
    while ((*REG_PTR(uart_base_vaddr, UARTFR) & PL011_UARTFR_TXFF) != 0);

    *REG_PTR(uart_base_vaddr, UARTDR) = ch;
    if (ch == '\r') {
        uart_put_char('\n');
    }
}


void uart_handle_irq() {
    *REG_PTR(uart_base_vaddr, UARTICR) = 0x7f0;
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
}

void notified(sel4cp_channel channel) {}
