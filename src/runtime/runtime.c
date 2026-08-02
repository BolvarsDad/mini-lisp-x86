/*
    mini-lisp-x86 - A compiler for a subset of Common Lisp to x86_64
    Copyright (C) 2025 Sinan Olsson-Pasic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    File: runtime.c
    Purpose: Freestanding process entry for compiled programs — no libc.
    _start calls the generated lisp_entry, prints its tagged-word result
    over raw write syscalls, and exits via the exit syscall, so programs
    link with `ld` alone: no crt startup files, no interpreter.
*/

#include <stdint.h>

#include "../compiler/ir.h"

extern int64_t lisp_entry(void);

/* x86_64 Linux syscall numbers */
#define SYS_WRITE 1
#define SYS_EXIT  60

#define STDOUT_FD 1

static void
sys_write(int fd, const void *buf, unsigned long count)
{
    long ret;

    /* the syscall instruction clobbers rcx (return rip) and r11 (rflags) */
    __asm__ volatile ("syscall"
                      : "=a"(ret)
                      : "a"((long)SYS_WRITE), "D"((long)fd), "S"(buf),
                        "d"(count)
                      : "rcx", "r11", "memory");
    (void)ret;
}

static void
sys_exit(int code)
{
    __asm__ volatile ("syscall"
                      :
                      : "a"((long)SYS_EXIT), "D"((long)code)
                      : "rcx", "r11", "memory");
    __builtin_unreachable();
}

static void
write_str(const char *s)
{
    unsigned long len = 0;

    while (s[len] != '\0')
        ++len;

    sys_write(STDOUT_FD, s, len);
}

static void
write_int(int64_t v)
{
    char buf[21];               /* -9223372036854775808 */
    char *p = buf + sizeof buf;
    /* negate as unsigned so INT64_MIN doesn't overflow */
    uint64_t u = (v < 0) ? -(uint64_t)v : (uint64_t)v;

    do {
        *--p = (char)('0' + u % 10);
        u /= 10;
    } while (u != 0);

    if (v < 0)
        *--p = '-';

    sys_write(STDOUT_FD, p, (unsigned long)(buf + sizeof buf - p));
}

static void
write_hex(uint64_t v)
{
    char buf[18];               /* "0x" + 16 digits */
    char *p = buf + sizeof buf;

    do {
        *--p = "0123456789abcdef"[v & 0xF];
        v >>= 4;
    } while (v != 0);

    *--p = 'x';
    *--p = '0';

    sys_write(STDOUT_FD, p, (unsigned long)(buf + sizeof buf - p));
}

void
runtime_print(int64_t obj)
{
    if (IS_INTEGER(obj)) {
        write_int(obj >> INTEGER_SHIFT);
    } else if (IS_NIL(obj)) {
        write_str("NIL");
    } else if (IS_T(obj)) {
        write_str("T");
    } else if (IS_STRING(obj)) {
        write_str((const char *)(obj & ~(int64_t)STRING_TAG));
    } else {
        write_str("#<unknown object type ");
        write_hex((uint64_t)obj);
        write_str(">");
    }
}

/* The kernel enters _start with rsp 16-aligned; compiled C assumes the
 * post-call alignment (8 mod 16), so realign before making any calls. */
__attribute__((force_align_arg_pointer))
void
_start(void)
{
    runtime_print(lisp_entry());
    sys_write(STDOUT_FD, "\n", 1);
    sys_exit(0);
}
