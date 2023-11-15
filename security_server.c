#include <microkit.h>

#define PROXY_CHANNEL 0

void notified(microkit_channel ch) {
}

microkit_msginfo protected(microkit_channel ch, microkit_msginfo message) {
    if (microkit_msginfo_get_label(message) != 0 ||
        microkit_mr_get(0) != 0 || microkit_mr_get(1) != 1 || microkit_mr_get(2) != 2) {
        
        microkit_dbg_puts("Unexpected ppcall recieved!");
    }
    message = microkit_msginfo_new(1, 0);
    return message;
}

void init() {
	// uart_init();
	// uart_put_
    // str("Hi! I'm PONG!\n");
    // microkit_notify(300);
    // arm_sys_null(seL4_SysDebugEnterKGDB);
}