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
    _start calls the generated lisp_entry and exits via the exit
    syscall, so programs link with `ld` alone: no crt startup files, no
    interpreter. Also provides the writers that compiled `format` calls.
*/

#include <stdint.h>

#include "../compiler/ir.h"

/* Both symbols are provided by the generated assembly: the entry point
 * itself, and a flag telling us whether to echo its result (see
 * codegen.h). Nothing else may be undefined in this object -- `nm -u
 * runtime.o` listing anything more means libc crept back in. */
extern int64_t lisp_entry(void);
extern const int64_t lisp_echo_result;

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

/*
 * Entry points for compiled `format`. The compiler expands a control
 * string into a sequence of these, one tagged word per call, so there
 * is no variadic convention to honour (see ir.h).
 */

/* The compiler only ever passes a string literal it emitted itself. */
void
runtime_write_str(int64_t obj)
{
    write_str((const char *)DECODE_STRING(obj));
}

/* ~a prints exactly what the result echo prints, so there is one
 * printer rather than two that can drift apart. */
void
runtime_write_aesthetic(int64_t obj)
{
    runtime_print(obj);
}

void
runtime_write_decimal(int64_t obj)
{
    /* Common Lisp signals a type error when ~d gets a non-integer.
     * With no condition system, printing it aesthetically is the
     * pragmatic choice -- a deliberate deviation. */
    if (IS_INTEGER(obj))
        write_int(obj >> INTEGER_SHIFT);
    else
        runtime_print(obj);
}

/* The kernel enters _start with rsp 16-aligned; compiled C assumes the
 * post-call alignment (8 mod 16), so realign before making any calls. */
__attribute__((force_align_arg_pointer))
void
_start(void)
{
    int64_t result = lisp_entry();

    /* Off unless the program was compiled with -d: otherwise the
     * runtime would append its own line to whatever `format` wrote. */
    if (lisp_echo_result) {
        runtime_print(result);
        sys_write(STDOUT_FD, "\n", 1);
    }

    sys_exit(0);
}
