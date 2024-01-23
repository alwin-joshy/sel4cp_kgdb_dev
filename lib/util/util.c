/*
 * Copyright 2022, UNSW (ABN 57 195 873 179)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "include/util.h"

static char hexchars[] = "0123456789abcdef";

/* This is required to use the printf library we brought in, it is
   simply for convenience since there's a lot of logging/debug printing
   in the VMM. */
void _putchar(char character)
{
    microkit_dbg_putc(character);
}

 __attribute__ ((__noreturn__))
void __assert_func(const char *file, int line, const char *function, const char *str)
{
    microkit_dbg_puts("assert failed: ");
    microkit_dbg_puts(str);
    microkit_dbg_puts(" ");
    microkit_dbg_puts(file);
    microkit_dbg_puts(" ");
    microkit_dbg_puts(function);
    microkit_dbg_puts("\n");
    while (1) {}
}

/* Convert a character (representing a hexadecimal) to its integer equivalent */
int hex(unsigned char c)
{
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

/* Convert a buffer to a hexadecimal string */
char *mem2hex(char *mem, char *buf, int size) {
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
char *hex2mem(char *buf, char *mem, int size) {
    int i;
    unsigned char c;

    for (i = 0; i < size; i++, mem++) {
        c = hex(*buf++) << 4;
        c += hex(*buf++);
        *mem = c;
    }
    return buf;
}
