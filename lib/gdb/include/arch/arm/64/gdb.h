//
// Created by Alwin Joshy on 23/1/2024.
//

#pragma once

#include "gdb.h"
#include <sel4/constants.h>

/* Convert registers to a hex string */
char *regs2hex(seL4_UserContext *regs, char *buf);

/* Convert registers to a hex string */
char *hex2regs(seL4_UserContext *regs, char *buf);

bool set_software_breakpoint(inferior_t *inferior, seL4_Word address);
bool unset_software_breakpoint(inferior_t *inferior, seL4_Word address);

bool set_hardware_breakpoint(inferior_t *inferior, seL4_Word address);
bool unset_hardware_breakpoint(inferior_t *inferior, seL4_Word address);

bool set_hardware_watchpoint(inferior_t *inferior, seL4_Word address,
                               seL4_BreakpointAccess type);
bool unset_hardware_watchpoint(inferior_t *inferior, seL4_Word address,
                               seL4_BreakpointAccess type);

bool enable_single_step(inferior_t *inferior);
bool disable_single_step(inferior_t *inferior);