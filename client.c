#include <microkit.h>

#define PMU_WRITE(reg, v)                      \
    do {                                       \
        seL4_Word _v = v;                         \
        asm volatile("msr  " reg ", %0" :: "r" (_v)); \
    }while(0)

#define PMU_READ(reg, v) asm volatile("mrs %0, " reg :  "=r"(v))
#define FASTFN inline __attribute__((always_inline))

#define PMUSERENR   "PMUSERENR_EL0"
#define PMINTENCLR  "PMINTENCLR_EL1"
#define PMINTENSET  "PMINTENSET_EL1"
#define PMCR        "PMCR_EL0"
#define PMCNTENCLR  "PMCNTENCLR_EL0"
#define PMCNTENSET  "PMCNTENSET_EL0"
#define PMXEVCNTR   "PMXEVCNTR_EL0"
#define PMSELR      "PMSELR_EL0"
#define PMXEVTYPER  "PMXEVTYPER_EL0"
#define PMCCNTR     "PMCCNTR_EL0"

#define BIT(n) (UL_CONST(1) << (n))
#define UL_CONST(x) x

#define ARMV8A_PMCR_N(x)       (((x) & 0xFFFF) >> 11u)
#define ARMV8A_PMCR_ENABLE     BIT(0)
#define ARMV8A_PMCR_RESET_ALL  BIT(1)
#define ARMV8A_PMCR_RESET_CCNT BIT(2)
#define ARMV8A_PMCR_DIV64      BIT(3) /* Should CCNT be divided by 64? */
#define ARMV8A_COUNTER_CCNT 31

static FASTFN void private_write_pmcr(uint32_t val)
{
    PMU_WRITE(PMCR, val);
}

static FASTFN uint32_t private_read_pmcr(void)
{
    uint32_t val;
    PMU_READ(PMCR, val);
    return val;
}

#define MODIFY_PMCR(op, val) private_write_pmcr(private_read_pmcr() op (val))

#define READ_CCNT(x) asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(x));


#define SERVER_CHANNEL 0

uintptr_t uart_base_vaddr;

#define UART_OFFSET 0x4c0
#define UART_WFIFO  0x0
#define UART_RFIFO  0x4
#define UART_CTRL 0x8
#define UART_STATUS 0xC

#define UART_TX_FULL        (1 << 21)
#define UART_RX_EMPTY       (1 << 20)
#define UART_CONTROL_TX_ENABLE   (1 << 12)

#define REG_PTR(base, offset) ((volatile uint32_t *)((base) + (offset)))

int a = 1;

void uart_init() {
    *REG_PTR(uart_base_vaddr + UART_OFFSET, UART_CTRL) |= UART_CONTROL_TX_ENABLE;
}

void uart_put_char(int ch) {
    while ((*REG_PTR(uart_base_vaddr + UART_OFFSET, UART_STATUS) & UART_TX_FULL));

    /* Add character to the buffer. */
    *REG_PTR(uart_base_vaddr + UART_OFFSET, UART_WFIFO) = ch;
    if (ch == '\n') {
        uart_put_char('\r');
    }
}

void uart_put_str(char *str) {
    while (*str) {
        uart_put_char(*str);
        str++;
    }
}
static char
hexchar(unsigned int v)
{
    return v < 10 ? '0' + v : ('a' - 10) + v;
}

void
puthex64(uint64_t val)
{
    char buffer[16 + 3];
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[16 + 3 - 1] = 0;
    for (unsigned i = 16 + 1; i > 1; i--) {
        buffer[i] = hexchar(val & 0xf);
        val >>= 4;
    }
    uart_put_str(buffer);
}

void init() {
	uart_init();
	uart_put_str("Hi! starting the client!\n");

	/* Set up the PMU */
	// Ensure all counters are in the stopped state
    PMU_WRITE(PMCNTENCLR, -1);

    //Clear div 64 flag
    MODIFY_PMCR(&, ~ARMV8A_PMCR_DIV64);

    //Reset all counters
    MODIFY_PMCR( |, ARMV8A_PMCR_RESET_ALL | ARMV8A_PMCR_RESET_CCNT);

    //Enable counters globally.
    MODIFY_PMCR( |, ARMV8A_PMCR_ENABLE);

    //start CCNT	
    PMU_WRITE(PMCNTENSET, BIT(ARMV8A_COUNTER_CCNT));


	/* Wait for the other components to be initialized */
	for (int i = 0; i < 100; i++) {
		seL4_Yield();
	}

	uint64_t start, end;
	uart_put_str("## BEGIN benchmark results ##\n");
	uart_put_str("proxy benchmark with security server\n");
	for (int i = 0; i < 100; i++) {
		microkit_mr_set(0, 0);
		microkit_mr_set(1, 1);
		microkit_mr_set(2, 2);
		microkit_msginfo msginfo = microkit_msginfo_new(0, 3);
		READ_CCNT(start);
		msginfo = microkit_ppcall(SERVER_CHANNEL, msginfo);
		READ_CCNT(end);
		puthex64(end - start);
		uart_put_str(", ");

		if (microkit_msginfo_get_label(msginfo) != 0 ||
			microkit_mr_get(0) != 2 || microkit_mr_get(1) != 1 || microkit_mr_get(2) != 0) {
			
			uart_put_str("Unexpected reply!\n");
		}
		// } else {
			// uart_put_str("all is gut!\n");
		// }

	}
	uart_put_str("\n## END benchmark results ##\n");
}

void notified(microkit_channel ch) {
	// switch (ch) {
	// case PINGPONG_CHANNEL:
	// 	uart_put_str("Ping!\n");
	// 	microkit_notify(PINGPONG_CHANNEL);
	// 	break;
	// }
}
