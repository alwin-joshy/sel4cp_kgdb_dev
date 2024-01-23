//
// Created by Alwin Joshy on 22/1/2024.
//
#pragma once

#include <uart.h>
#include <sel4/sel4_arch/types.h>
#include <microkit.h>



#define MAX_ELF_NAME 32
#define MAX_SW_BREAKS 10

/* Bookkeeping for watchpoints */
typedef struct watchpoint {
    uint64_t addr;
    seL4_BreakpointAccess type;
} hw_watch_t;

/* Bookkeeping for hardware breakpoints */
typedef struct hw_breakpoint {
    uint64_t addr;
} hw_break_t;

/* Bookkeeping for software breakpoints */
typedef struct sw_breakpoint {
    uint64_t addr;
    uint32_t orig_instr;
} sw_break_t;

/* GDB uses 'inferiors' to distinguish between different processes (in our case PDs) */
typedef struct inferior {
    microkit_id micro_id;
    /* The id in GDB cannot be 0, because this has a special meaning in GDB */
    uint16_t gdb_id;
    seL4_CPtr tcb;
    char elf_name[MAX_ELF_NAME];
    sw_break_t software_breakpoints[MAX_SW_BREAKS];
    hw_break_t hardware_breakpoints[seL4_NumExclusiveBreakpoints];
    hw_watch_t hardware_watchpoints[seL4_NumExclusiveWatchpoints];
    bool ss_enabled;
} inferior_t;

int gdb_register_initial(microkit_id id, char* elf_name);
int gdb_register_inferior(microkit_id id, char *elf_name);
void gdb_event_loop();

