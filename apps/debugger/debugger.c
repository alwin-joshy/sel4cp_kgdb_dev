#include <uart.h>
#include <sel4/sel4_arch/types.h>
#include <microkit.h>
#include <gdb.h>

void init() {
	uart_init();

    /* Register the first protection domain */
    if (gdb_register_initial(0, "ping.elf")) {
        uart_put_str("Failed to initialize initial inferior");
        return;
    }

    /* Wait for a connection to be established */
    gdb_event_loop();

    /* Register any remaining protection domains */
    if (gdb_register_inferior(1, "pong.elf")) {
        uart_put_str("Failed to initialize other inferior");
        return;
    }
}

void fault(microkit_channel ch, microkit_msginfo msginfo) {
    gdb_handle_fault(ch, msginfo);
}

void notified(microkit_channel ch) {
    gdb_event_loop();
}