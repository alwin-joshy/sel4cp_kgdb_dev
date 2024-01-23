#include "lib/uart/include/uart.h"

#define BUFSIZE 1024
#define MAX_PDS 64
#define MAX_ELF_NAME 32
#define MAX_ID 255
#define INITIAL_INFERIOR_POS 0

/* Input buffer */
static char input[BUFSIZE];

/* Output buffer */
static char output[BUFSIZE];

int num_threads = 0;
inferior_t inferiors[MAX_PDS];
inferior_t *target_inferior = NULL;

static char *kgdb_get_packet(void) {
    char c;
    int count;
    /* Checksum and expected checksum */
    unsigned char cksum, xcksum;
    char *buf = input;
    (void) buf;

    while (1) {
        /* Wait for the start character - ignoring all other characters */
        while ((c = uart_get_char()) != '$')
#ifndef DEBUG_PRINTS
            ;
#else
        {
            uart_put_char(c);
        }
        uart_put_char(c);
#endif
retry:
        /* Initialize cksum variables */
        cksum = 0;
        xcksum = -1;
        count = 0;
        (void) xcksum;

        /* Read until we see a # or the buffer is full */
        while (count < BUFSIZE - 1) {
            c = uart_get_char();
#ifdef DEBUG_PRINTS
            uart_put_char(c);
#endif
            if (c == '$') {
                goto retry;
            } else if (c == '#') {
                break;
            }
            cksum += c;
            buf[count++] = c;
        }

        /* Null terminate the string */
        buf[count] = 0;

#ifdef DEBUG_PRINTS
        printf("\nThe value of the command so far is %s. The checksum you should enter is %x\n", buf, cksum);
#endif

        if (c == '#') {
            c = uart_get_char();
            xcksum = hex(c) << 4;
            c = uart_get_char();
            xcksum += hex(c);

            if (cksum != xcksum) {
                uart_put_char('-');   /* checksum failed */
            } else {
                uart_put_char('+');   /* checksum success, ack*/

                if (buf[2] == ':') {
                    uart_put_char(buf[0]);
                    uart_put_char(buf[1]);

                    return &buf[3];
                }

                return buf;
            }
        }
    }

    return NULL;
}

/* Read registers */
static void handle_read_regs(void) {
    seL4_UserContext context;
    register_set_t regs;
    int error = seL4_TCB_ReadRegisters(target_thread->tcb, false, 0,
                                       sizeof(seL4_UserContext) / sizeof(seL4_Word), &context);
    regs2hex(&regs, output);
}

static void handle_write_regs(char *ptr) {
    assert(*ptr++ == 'G');

    seL4_UserContext regs;
    hex2regs(&regs, ptr);
    int error = seL4_TCB_WriteRegisters(target_thread->tcb, false, 0,
                                       sizeof(seL4_UserContext) / sizeof(seL4_Word), &context);
    strlcpy(kgdb_out, "OK", sizeof(kgdb_out));
}

static void handle_query(char *ptr) {
    if (strncmp(ptr, "qSupported", 10) == 0) {
        /* TODO: This may eventually support more features */
        snprintf(kgdb_out, sizeof(kgdb_out),
                 "qSupported:PacketSize=%lx;QThreadEvents+;swbreak+;hwbreak+;vContSupported+;fork-events+;exec-events+;multiprocess+;", sizeof(kgdb_in));
    } else if (strncmp(ptr, "qfThreadInfo", 12) == 0) {
        char *out_ptr = kgdb_out;
        *out_ptr++ = 'm';
        for (uint8_t i = 0; i < 64; i++) {
            if (threads[i].tcb != NULL) {
                if (i != 0) {
                    *out_ptr++ = ',';
                }
                *out_ptr++ = 'p';
                out_ptr = k_mem2hex((char *) &threads[i].gdb_id, out_ptr, sizeof(uint8_t));
                strlcpy(out_ptr, ".1", 3);
                /* @alwin: this is stupid, be better */
                out_ptr += 2;
            } else {
                break;
            }
        }
    } else if (strncmp(ptr, "qsThreadInfo", 12) == 0) {
        strlcpy(kgdb_out, "l", sizeof(kgdb_out));
    } else if (strncmp(ptr, "qC", 2) == 0) {
        strlcpy(kgdb_out, "QCp1.1", sizeof(kgdb_out));
    } else if (strncmp(ptr, "qSymbol", 7) == 0) {
        strlcpy(kgdb_out, "OK", sizeof(kgdb_out));
    } else if (strncmp(ptr, "qTStatus", 8) == 0) {
        /* TODO: THis should eventually work in the non startup case */
        strlcpy(kgdb_out, "T0", sizeof(kgdb_out));
    } else if (strncmp(ptr, "qAttached", 9) == 0) {
        strlcpy(kgdb_out, "1", sizeof(kgdb_out));
    }
}

static void handle_configure_debug_events(char *ptr) {
    seL4_Word addr, size;
    bool_t success;

    if (!parse_breakpoint_format(ptr, &addr, &size)) {
        strlcpy(kgdb_out, "E01", sizeof(kgdb_out));
        return;
    }

    /* Breakpoints and watchpoints */

    if (strncmp(ptr, "Z0", 2) == 0) {
        /* Set a software breakpoint using binary rewriting */
        success = set_software_breakpoint(target_thread, addr);
    } else if (strncmp(ptr, "z0", 2) == 0) {
        /* Unset a software breakpoint */
        success = unset_software_breakpoint(target_thread, addr);
    } else if (strncmp(ptr, "Z1", 2) == 0) {
        /* Set a hardware breakpoint */
        success = set_hardware_breakpoint(target_thread, addr);
    } else if (strncmp(ptr, "z1", 2) == 0) {
        /* Unset a hardware breakpoint */
        success = unset_hardware_breakpoint(target_thread, addr);
    } else if (strncmp(ptr, "Z2", 2) == 0) {
        /* Set a write watchpoint */
        success = set_watchpoint(target_thread, addr, WATCHPOINT_WRITE);
    } else if (strncmp(ptr, "z2", 2) == 0) {
        /* Unset a write watchpoint */
        success = unset_watchpoint(target_thread, addr);
    } else if (strncmp(ptr, "Z3", 2) == 0) {
        /* Set a read watchpoint */
        success = set_watchpoint(target_thread, addr, WATCHPOINT_READ);
    } else if (strncmp(ptr, "z3", 2) == 0) {
        /* Unset a read watchpoint */
        success = unset_watchpoint(target_thread, addr);
    } else if (strncmp(ptr, "Z4", 2) == 0) {
        /* Set an access watchpoint */
        success = set_watchpoint(target_thread, addr, WATCHPOINT_ACCESS);
    } else if (strncmp(ptr, "z4", 2) == 0) {
        /* Unset an access watchpoint */
        success = unset_watchpoint(target_thread, addr);
    }

    if (!success) {
        strlcpy(kgdb_out, "E01", sizeof(kgdb_out));
    } else {
        strlcpy(kgdb_out, "OK", sizeof(kgdb_out));
    }
}


void gdb_event_loop() {
    char *ptr;

    while (1) {
        ptr = gdb_get_packet();
        output[0] = 0;
        if (*ptr == 'g') {
            handle_read_regs();
        } else if (*ptr == 'G') {
            handle_write_regs(ptr);
        } else if (*ptr == 'm') {
            handle_read_mem(ptr);
        } else if (*ptr == 'M') {
            handle_write_mem(ptr);
        } else if (*ptr == 'c' || *ptr == 's') {
            int stepping = *ptr == 's' ? 1 : 0;
            ptr++;

            if (stepping) {
                enable_single_step();
            } else {
                disable_single_step();
            }
            /* TODO: Support continue from an address and single step */
            break;
        } else if (*ptr == 'q') {
            handle_query(ptr);
        } else if (*ptr == 'H') {
            handle_set_thread(ptr);
        } else if (*ptr == '?') {
            /* TODO: This should eventually report more reasons than swbreak */
            strlcpy(kgdb_out, "T05swbreak:;", sizeof(kgdb_out));
        } else if (*ptr == 'v') {
            if (strncmp(ptr, "vCont?", 7) == 0) {
                strlcpy(kgdb_out, "vCont;c", sizeof(kgdb_out));
            } else if (strncmp(ptr, "vCont;c", 7) == 0) {
                break;
            }
        } else if (*ptr == 'z' || *ptr == 'Z') {
            handle_configure_debug_events(ptr);
        }

        kgdb_put_packet(kgdb_out);
    }
}

int gdb_register_initial(microkit_id id, char* elf_name) {
    /* If this isn't the first thread that was initialized */
    if (num_threads != 0 || threads[num_threads].tcb != 0) {
        return -1;
    }

    /* If the provided id is greater than expected */
    // @alwin: is this right?
    if (pd > MAX_ID) {
        return -1;
    }

    inferiors[0].micro_id = id;
    inferiors[0].gdb_id = 1;
    inferiors[0].tcb = BASE_TCB_CAP + id;
    strncpy(inferiors[0].elf_name, elf_name, MAX_ELF_NAME);
    target_inferior = &inferiors[0];
    num_threads = 1;
    return 0;
}

void gdb_register_inferior(microkit_id id, char *elf_name) {
    /* Must already have one thread that has been registered */
    if (num_threads < 1 || inferiors[0].tcb == 0) {
        return -1;
    }

    /* We have too many PDs */
    if (num_threads >= MAX_PDS) {
        return -1;
    }

    uint8_t idx = num_threads++;
    uint16_t gdb_id = idx + 1;
    inferiors[idx].micro_id = id;
    inferiors[idx].gdb_id = gdb_id;
    strncpy(inferiors[idx].elf_name, elf_name, MAX_ELF_NAME);

    /* Indicate that the initial thread has forked */
    inferiors[idx].tcb = inferiors[INITIAL_INFERIOR_POS].tcb;
    strlcpy(output, "T05fork:p", sizeof(output));
    char *buf = mem2hex((char *) &gdb_id, output + strnlen(output, BUFSIZE), sizeof(uint8_t));
    strlcpy(buf, ".1;", BUFSIZE - strnlen(output, BUFSIZE));
    gdb_put_packet(output);
    gdb_event_loop();

    /* Indicate that the new thread is execing something */
    strlcpy(output, "T05exec:", BUFSIZE);
    buf = mem2hex(elf_name, output + strnlen(output, BUFSIZE), BUFSIZE - strnlen(output, BUFSIZE));
    strlcpy(buf, ";", BUFSIZE - strnlen(output, BUFSIZE));
    inferiors[idx].tcb = BASE_TCB_CAP + id;
    gdb_put_packet(output);
    gdb_event_loop();
}

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