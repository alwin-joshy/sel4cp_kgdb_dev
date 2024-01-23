#include <uart/uart.h>
#include <sel4/sel4_arch/types.h>
#include <microkit.h>

void init() {
	uart_init();

    /* Register the first protection domain */
    if (gdb_register_initial(0, "ping.elf")) {
        printf("Failed to initialize initial inferior")
        return;
    }

    /* Wait for a connection to be established */
    gdb_event_loop();

    /* Register any remaining protection domains */
    if (gdb_register_inferior(1, "pong.elf")) {
        printf("Failed to initialize other inferior")
        return;
    }
}

void fault() {

}

void notified() {
    gdb_event_loop();
}