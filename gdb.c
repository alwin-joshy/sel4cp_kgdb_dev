#include "include/uart.h"

#define BUFSIZE 1024
#define MAX_PDS 64
#define MAX_ELF_NAME 32
#define MAX_ID 255
#define INITIAL_INFERIOR_POS 0

/* Input buffer */
static char input[BUFSIZE];

/* Output buffer */
static char output[BUFSIZE];

/* GDB uses 'inferiors' to distinguish between different processes (in our case PDs) */
typedef struct inferior {
    microkit_id micro_id;
    /* The id in GDB cannot be 0, because this has a special meaning in GDB */
    uint16_t gdb_id;
    seL4_CPtr tcb;
    char elf_name[MAX_ELF_NAME];
} inferior_t;


int num_threads = 0;
inferior_t inferiors[MAX_PDS];
inferior_t *target_inferior = NULL;


/* Convert a buffer to a hexadecimal string */
static char *mem2hex(char *mem, char *buf, int size) {
    int i;
    unsigned char c;
    for (i = 0; i < size; i++, mem++) {
        c = *mem;
        *buf++ = hexchars[c >> 4];
        *buf++ = hexchars[c % 16];
    }
    *buf = 0;
    return buf;
}

/* Fill a buffer based with the contents of a hex string */
static char *hex2mem(char *buf, char *mem, int size) {
    int i;
    unsigned char c;

    for (i = 0; i < size; i++, mem++) {
        c = hex(*buf++) << 4;
        c += hex(*buf++);
        *mem = c;
    }
    return buf;
}

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

typedef struct register_set {
    uint64_t registers_64[NUM_REGS - 1];
    uint32_t cpsr;
} register_set_t;


/* Convert registers to a hex string */
// @alwin: This is rather unpleasant, but the way the seL4_UserContext struct is formatted is annoying
char *regs2hex(seL4_UserContext *regs, char *buf)
{
    /* First we handle the 64 bit general purpose registers*/
    buf = mem2hex((char *) &regs->x0, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x1, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x2, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x3, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x4, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x5, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x7, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x8, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x9, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x10, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x11, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x12, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x13, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x14, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x15, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x16, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x17, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x18, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x19, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x20, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x21, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x22, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x23, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x24, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x25, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x26, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x27, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x28, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x29, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->x30, buf, sizeof(seL4_Word));

    /* Now the stack pointer and the instruction pointer */
    buf = mem2hex((char *) &regs->sp, buf, sizeof(seL4_Word));
    buf = mem2hex((char *) &regs->pc, buf, sizeof(seL4_Word));

    /* Finally the cpsr */
    return mem2hex((char *) &regs->spsr, buf, sizeof(seL4_Word) / 2);
}

/* Convert registers to a hex string */
// @alwin: This is rather unpleasant, but the way the seL4_UserContext struct is formatted is annoying
char *hex2regs(seL4_UserContext *regs, char *buf)
{
    /* First we handle the 64 bit general purpose registers*/
    buf = hex2mem((char *) buf, &regs->x0, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x1, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x2, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x3, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x4, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x5, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x6, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x7, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x8, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x9, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x10, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x11, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x12, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x13, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x14, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x15, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x16, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x17, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x18, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x19, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x20, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x21, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x22, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x23, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x24, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x25, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x26, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x27, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x28, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x29, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->x30, sizeof(seL4_Word));

    /* Now the stack pointer and the instruction pointer */
    buf = hex2mem((char *) buf, &regs->sp, sizeof(seL4_Word));
    buf = hex2mem((char *) buf, &regs->pc, sizeof(seL4_Word));

    /* Finally the cpsr */
    buf = hex2mem((char *) buf, &regs->spsr, sizeof(seL4_Word) / 2);
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
    seL4_UserContext regs;
    hex2regs(&regs, ptr);
    int error = seL4_TCB_WriteRegisters(target_thread->tcb, false, 0,
                                       sizeof(seL4_UserContext) / sizeof(seL4_Word), &context);
    strlcpy(kgdb_out, "OK", sizeof(kgdb_out));
}


void gdb_event_loop() {
    char *ptr;

    while (1) {
        ptr = gdb_get_packet();
        output[0] = 0;
        if (*ptr == 'g') {
            handle_read_regs();
        } else if (*ptr == 'G') {
            handle_write_regs(ptr++);
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