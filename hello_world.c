#include <sel4cp.h>


void init() {
	uart_init();
	uart_put_str("HELLO WORLD!\n");
    arm_sys_null(seL4_SysDebugEnterKGDB);
}

void notified(sel4cp_channel channel) {}
