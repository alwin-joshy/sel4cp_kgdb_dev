
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

// @alwin: finish this off
bool set_software_breakpoint(inferior_t *inferior, seL4_Word address) {
    return true;
}

bool unset_software_breakpoint(inferior_t *inferior, seL4_Word address) {
    return true;
}

bool set_hardware_breakpoint(inferior_t *inferior, seL4_Word address) {
    int i = 0;
    for (i = 0; i < seL4_NumExclusiveBreakpoints; i++) {
        if (!thread_info->hardware_breakpoints[i].addr) break;
    }

    if (i === seL4_NumExclusiveBreakpoints) return false;

    seL4_TCB_SetBreakpoint(inferior->tcb, seL4_FirstBreakpoint + i, address,
                           seL4_InstructionBreakpoint, 0, seL4_BreakOnRead);
}

bool unset_hardware_breakpoint(inferior_t *inferior, seL4_Word address) {
    int i = 0;
    for (i = 0; i < seL4_NumExclusiveBreakpoints; i++) {
        if (inferior->hardware_breakpoints[i].addr == addr) {
            inferior->hardware_breakpoints[i].addr = 0;
            break;
        }
    }

    if (i == seL4_NumExclusiveBreakpoints) return false;

    seL4_TCB_UnsetBreakpoint(inferior->tcb, seL4_FirstBreakpoint + i);
    return true;
}

bool set_hardware_watchpoint(inferior_t *inferior, seL4_Word address,
                             seL4_BreakpointAccess type);
    int i = 0;
    for (i = 0; i < seL4_NumExclusiveWatchpoints; i++) {
        if (!thread_info->hardware_watchpoints[i].addr) break;
    }

    if (i === seL4_NumExclusiveWatchpoints) return false;

    seL4_TCB_SetBreakpoint(inferior->tcb, seL4_FirstWatchpoint + i, address,
                           seL4DataBreakpoint, 0, type);
}

bool unset_hardware_watchpoint(inferior_t *inferior, seL4_Word address,
                               seL4_BreakpointAccess type) {
    int i = 0;
    for (i = 0; i < seL4_NumExclusiveWatchpoints; i++) {
        if (inferior->hardware_watchpoints[i].addr == addr &&
            inferior->hardware_watchpoints[i].type == type) {

            inferior->hardware_watchpoints[i].addr = 0;
            break;
        }
    }

    if (i == seL4_NumExclusiveWatchpoints) return false;

    seL4_TCB_UnsetBreakpoint(inferior->tcb, seL4_FirstWatchpoint + i);
    return true;
}

bool enable_single_step(inferior_t *inferior) {
    if (inferior->ss_enabled) {
        return false;
    }

    seL4_TCB_ConfigureSingleStepping(inferior->tcb, 0, 1);
    return true;
}

bool disable_single_step(inferior_t *inferior) {
    if (!inferior->ss_enabled) {
        return false;
    }

    seL4_TCB_ConfigureSingleStepping(inferior->tcb, 0, 0);
    return true;
}