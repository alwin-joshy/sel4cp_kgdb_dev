#include <microkit.h>

#define CLIENT_CHANNEL 0
#define SERVER_CHANNEL 1

void init() {

}

microkit_msginfo protected(microkit_channel ch, microkit_msginfo message) {
    if (microkit_msginfo_get_label(message) != 0 ||
        microkit_mr_get(0) != 0 || microkit_mr_get(1) != 1 || microkit_mr_get(2) != 2) {
        
        microkit_dbg_puts("Unexpected ppcall recieved!");
    }
    message = microkit_ppcall(SERVER_CHANNEL, message);
    // microkit_mr_set(0, 2);
    // microkit_mr_set(1, 1);
    // microkit_mr_set(2, 0);
    return message;
}

void notified(microkit_channel ch) {
}