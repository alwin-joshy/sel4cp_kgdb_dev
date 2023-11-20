#include <microkit.h>

#define CLIENT_CHANNEL 0
#define SERVER_CHANNEL 1
#define SECURITY_CHANNEL 2


void init() {

}

microkit_msginfo protected(microkit_channel ch, microkit_msginfo message) {
    uint64_t arg0 = microkit_mr_get(0);
    uint64_t arg1 = microkit_mr_get(1);
    uint64_t arg2 = microkit_mr_get(2);

    if (microkit_msginfo_get_label(message) != 0 ||
        arg0 != 0 || arg1 != 1 || arg2 != 2) {
        
        microkit_dbg_puts("Unexpected ppcall recieved!");
    }

    microkit_msginfo security_message = microkit_ppcall(SECURITY_CHANNEL, message);
    if (microkit_msginfo_get_label(security_message) != 1) {
        return microkit_msginfo_new(-1, 0);
    }

    microkit_mr_set(0, arg0);
    microkit_mr_set(1, arg1);
    microkit_mr_set(2, arg2);
    message = microkit_ppcall(SERVER_CHANNEL, message);

    return message;
}

void notified(microkit_channel ch) {
}