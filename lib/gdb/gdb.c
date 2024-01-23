//
// Created by Alwin Joshy on 23/1/2024.
//

#include <gdb.h>
#include <arch/arm/64/gdb.h>
#include <util.h>
#include <string.h>

#define BUFSIZE 1024
#define MAX_PDS 64
#define MAX_ID 255
#define INITIAL_INFERIOR_POS 0

/* Input buffer */
static char input[BUFSIZE];

/* Output buffer */
static char output[BUFSIZE];

int num_threads = 0;
inferior_t inferiors[MAX_PDS];
inferior_t *target_inferior = NULL;

static char *gdb_get_packet(void) {
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
            xcksum = hexchar_to_int(c) << 4;
            c = uart_get_char();
            xcksum += hexchar_to_int(c);

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

/*
 * Send a packet, computing it's checksum, waiting for it's acknoledge.
 * If there is not ack, packet will be resent.
 */
static void gdb_put_packet(char *buf)
{
    uint8_t cksum;
    for (;;) {
        uart_put_char('$');
        for (cksum = 0; *buf; buf++) {
            cksum += *buf;
            uart_put_char(*buf);
        }
        uart_put_char('#');
        uart_put_char(int_to_hexchar(cksum >> 4));
        uart_put_char(int_to_hexchar(cksum % 16));
        if (uart_get_char() == '+') {
            break;
        }
    }
}

/* Read registers */
static void handle_read_regs(void) {
    seL4_UserContext context;
    int error = seL4_TCB_ReadRegisters(target_inferior->tcb, false, 0,
                                       sizeof(seL4_UserContext) / sizeof(seL4_Word), &context);
    regs2hex(&context, output);
}

/* Write registers */
static void handle_write_regs(char *ptr) {
    assert(*ptr++ == 'G');

    seL4_UserContext context;
    hex2regs(&context, ptr);
    int error = seL4_TCB_WriteRegisters(target_inferior->tcb, false, 0,
                                        sizeof(seL4_UserContext) / sizeof(seL4_Word), &context);
    strlcpy(output, "OK", sizeof(output));
}

static void handle_query(char *ptr) {
    if (strncmp(ptr, "qSupported", 10) == 0) {
        /* TODO: This may eventually support more features */
        snprintf(output, sizeof(output),
                 "qSupported:PacketSize=%lx;QThreadEvents+;swbreak+;hwbreak+;vContSupported+;fork-events+;exec-events+;multiprocess+;", sizeof(input));
    } else if (strncmp(ptr, "qfThreadInfo", 12) == 0) {
        char *out_ptr = output;
        *out_ptr++ = 'm';
        for (uint8_t i = 0; i < 64; i++) {
            if (inferiors[i].tcb != 0) {
                if (i != 0) {
                    *out_ptr++ = ',';
                }
                *out_ptr++ = 'p';
                out_ptr = mem2hex((char *) &inferiors[i].gdb_id, out_ptr, sizeof(uint8_t));
                strlcpy(out_ptr, ".1", 3);
                /* @alwin: this is stupid, be better */
                out_ptr += 2;
            } else {
                break;
            }
        }
    } else if (strncmp(ptr, "qsThreadInfo", 12) == 0) {
        strlcpy(output, "l", sizeof(output));
    } else if (strncmp(ptr, "qC", 2) == 0) {
        strlcpy(output, "QCp1.1", sizeof(output));
    } else if (strncmp(ptr, "qSymbol", 7) == 0) {
        strlcpy(output, "OK", sizeof(output));
    } else if (strncmp(ptr, "qTStatus", 8) == 0) {
        /* TODO: THis should eventually work in the non startup case */
        strlcpy(output, "T0", sizeof(output));
    } else if (strncmp(ptr, "qAttached", 9) == 0) {
        strlcpy(output, "1", sizeof(output));
    }
}

static bool parse_breakpoint_format(char *ptr, seL4_Word *addr, seL4_Word *kind)
{
    /* Parse the first three characters */
    assert (*ptr == 'Z' || *ptr == 'z');
    ptr++;
    assert (*ptr >= '0' && *ptr <= '4');
    ptr++;
    assert(*ptr++ == ',');

    /* Parse the addr and kind */

    *addr = 0;
    *kind = 0;

    ptr = hexstr_to_int(ptr, sizeof(seL4_Word) * 2, addr);
    if (*ptr++ != ',') {
        return false;
    }

    /* @alwin: whats this about again? */
    ptr = hexstr_to_int(ptr, sizeof(seL4_Word) * 2, kind);
    if (*kind != 4) {
        return false;
    }

    return true;
}


static void handle_configure_debug_events(char *ptr) {
    /* Precondition: ptr[0] is always 'z' or 'Z' */
    seL4_Word addr, size;
    bool success = false;

    if (!parse_breakpoint_format(ptr, &addr, &size)) {
        strlcpy(output, "E01", sizeof(output));
        return;
    }

    /* Breakpoints and watchpoints */

    if (strncmp(ptr, "Z0", 2) == 0) {
        /* Set a software breakpoint using binary rewriting */
        success = set_software_breakpoint(target_inferior, addr);
    } else if (strncmp(ptr, "z0", 2) == 0) {
        /* Unset a software breakpoint */
        success = unset_software_breakpoint(target_inferior, addr);
    } else if (strncmp(ptr, "Z1", 2) == 0) {
        /* Set a hardware breakpoint */
        success = set_hardware_breakpoint(target_inferior, addr);
    } else if (strncmp(ptr, "z1", 2) == 0) {
        /* Unset a hardware breakpoint */
        success = unset_hardware_breakpoint(target_inferior, addr);
    } else {
        seL4_BreakpointAccess watchpoint_type;
        switch (ptr[1]) {
            case '2':
                watchpoint_type = seL4_BreakOnWrite;
                break;
            case '3':
                watchpoint_type = seL4_BreakOnRead;
                break;
            case '4':
                watchpoint_type = seL4_BreakOnReadWrite;
                break;
            default:
                strlcpy(output, "E01", sizeof(output));
                return;
        }

        if (ptr[0] == 'Z') {
            success = set_hardware_watchpoint(target_inferior, addr, watchpoint_type);
        } else {
            success = unset_hardware_watchpoint(target_inferior, addr, watchpoint_type);
        }
    }

    if (!success) {
        strlcpy(output, "E01", sizeof(output));
    } else {
        strlcpy(output, "OK", sizeof(output));
    }
}

int gdb_register_initial(microkit_id id, char* elf_name) {
    /* If this isn't the first thread that was initialized */
    if (num_threads != 0 || inferiors[num_threads].tcb != 0) {
        return -1;
    }

    /* If the provided id is greater than expected */
    // @alwin: is this right?
    if (id > MAX_ID) {
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

int gdb_register_inferior(microkit_id id, char *elf_name) {
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

    return 0;
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
                enable_single_step(target_inferior);
            } else {
                disable_single_step(target_inferior);
            }
            /* TODO: Support continue from an address and single step */
            break;
        } else if (*ptr == 'q') {
            handle_query(ptr);
        } else if (*ptr == 'H') {
            handle_set_thread(ptr);
        } else if (*ptr == '?') {
            /* TODO: This should eventually report more reasons than swbreak */
            strlcpy(output, "T05swbreak:;", sizeof(output));
        } else if (*ptr == 'v') {
            if (strncmp(ptr, "vCont?", 7) == 0) {
                strlcpy(output, "vCont;c", sizeof(output));
            } else if (strncmp(ptr, "vCont;c", 7) == 0) {
                break;
            }
        } else if (*ptr == 'z' || *ptr == 'Z') {
            handle_configure_debug_events(ptr);
        }

        gdb_put_packet(output);
    }
}