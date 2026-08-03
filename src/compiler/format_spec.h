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

    File: format_spec.h
    Purpose: Public interface for the `format` control-string parser.
*/

#ifndef MINI_LISP_X86_SRC_COMPILER_FORMAT_SPEC_H_
#define MINI_LISP_X86_SRC_COMPILER_FORMAT_SPEC_H_

#include <stddef.h>

/*
 * Control strings are parsed at compile time, not interpreted at run
 * time: `format` requires a literal control string, which is expanded
 * into a straight-line sequence of one-argument runtime calls. Bad
 * directives are therefore compile errors with a source position.
 *
 * Semantic analysis parses the control string to validate it, and IR
 * translation parses it again to expand it, so the parser lives here
 * rather than in either stage.
 *
 * Supported directives (case-insensitive, as in Common Lisp):
 *   ~a  print the next argument the way the REPL would
 *   ~d  print the next argument as a decimal integer
 *   ~%  a newline
 *   ~~  a literal tilde
 */

enum format_seg_kind {
    FORMAT_SEG_LITERAL,    /* text to write verbatim */
    FORMAT_SEG_AESTHETIC,  /* ~a: print the next argument like the REPL does */
    FORMAT_SEG_DECIMAL     /* ~d: print the next argument as a decimal integer */
};

struct format_seg {
    enum format_seg_kind kind;
    char *text;            /* LITERAL only; owned, NUL-terminated */
};

struct format_spec {
    struct format_seg *segs;
    size_t count;
    size_t capacity;
    size_t directive_count;   /* segments that consume an argument */
};

#define FORMAT_SPEC_OK     0
#define FORMAT_SPEC_BAD    (-1)  /* unknown or unterminated directive */
#define FORMAT_SPEC_NOMEM  (-2)

/*
 * Parse `control` into `out`, which need not be initialized.
 *
 * `~%` and `~~` resolve into the surrounding literal run rather than
 * becoming segments of their own, so "Hello, World!~%" parses to a
 * single literal and compiles to a single call.
 *
 * On FORMAT_SPEC_BAD, `*bad_offset` is the byte offset of the offending
 * `~` within `control` and `*bad_directive` is the character following
 * it, or '\0' when the string ends after the `~`. `out` is left empty;
 * only a successful parse needs format_spec_free.
 */
int  format_spec_parse(const char *control, struct format_spec *out,
                       size_t *bad_offset, char *bad_directive);
void format_spec_free (struct format_spec *spec);

#endif
