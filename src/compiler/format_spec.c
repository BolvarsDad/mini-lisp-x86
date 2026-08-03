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

    File: format_spec.c
    Purpose: Implements the `format` control-string parser shared by
    semantic analysis and IR translation.
*/

#include <ctype.h>
#include <stdlib.h>

#include "format_spec.h"

#define SEG_INITIAL_CAPACITY 8
#define LIT_INITIAL_CAPACITY 32

/* Accumulates the literal run between two directives. Its buffer is
 * handed to the segment on flush, so ownership leaves the struct. */
struct litbuf {
    char *data;
    size_t len;
    size_t capacity;
};

static int  spec_push_seg (struct format_spec *spec, enum format_seg_kind kind, char *text);
static int  litbuf_push   (struct litbuf *buf, char c);
static int  litbuf_flush  (struct litbuf *buf, struct format_spec *spec);

static int
spec_push_seg(struct format_spec *spec, enum format_seg_kind kind, char *text)
{
    if (spec->count >= spec->capacity) {
        size_t capacity = spec->capacity == 0
            ? SEG_INITIAL_CAPACITY
            : spec->capacity * 2;

        struct format_seg *grown = realloc(spec->segs, capacity * sizeof(*grown));
        if (grown == NULL)
            return -1;

        spec->segs = grown;
        spec->capacity = capacity;
    }

    spec->segs[spec->count].kind = kind;
    spec->segs[spec->count].text = text;
    spec->count++;

    if (kind != FORMAT_SEG_LITERAL)
        spec->directive_count++;

    return 0;
}

static int
litbuf_push(struct litbuf *buf, char c)
{
    if (buf->len + 1 >= buf->capacity) { // + 1 keeps room for the NUL
        size_t capacity = buf->capacity == 0
            ? LIT_INITIAL_CAPACITY
            : buf->capacity * 2;

        char *grown = realloc(buf->data, capacity);
        if (grown == NULL)
            return -1;

        buf->data = grown;
        buf->capacity = capacity;
    }

    buf->data[buf->len++] = c;

    return 0;
}

// An empty run produces no segment; a non-empty one gives its buffer away.
static int
litbuf_flush(struct litbuf *buf, struct format_spec *spec)
{
    if (buf->len == 0)
        return 0;

    buf->data[buf->len] = '\0';

    if (spec_push_seg(spec, FORMAT_SEG_LITERAL, buf->data) != 0)
        return -1;

    buf->data = NULL;
    buf->len = 0;
    buf->capacity = 0;

    return 0;
}

int
format_spec_parse(const char *control, struct format_spec *out,
                  size_t *bad_offset, char *bad_directive)
{
    struct litbuf lit = { NULL, 0, 0 };
    int status = FORMAT_SPEC_OK;
    size_t i = 0;

    if (control == NULL || out == NULL)
        return FORMAT_SPEC_BAD;

    out->segs = NULL;
    out->count = 0;
    out->capacity = 0;
    out->directive_count = 0;

    while (control[i] != '\0' && status == FORMAT_SPEC_OK) {
        if (control[i] != '~') {
            if (litbuf_push(&lit, control[i]) != 0)
                status = FORMAT_SPEC_NOMEM;

            ++i;
            continue;
        }

        // control[i] is '~', so control[i + 1] is in bounds
        char directive = control[i + 1];

        switch (tolower((unsigned char)directive)) {
            case '%':
                // Newlines and escaped tildes resolve into the
                // surrounding run instead of becoming their own
                // segment, so "text~%" stays a single call
                if (litbuf_push(&lit, '\n') != 0)
                    status = FORMAT_SPEC_NOMEM;
                break;

            case '~':
                if (litbuf_push(&lit, '~') != 0)
                    status = FORMAT_SPEC_NOMEM;
                break;

            case 'a':
            case 'd': {
                enum format_seg_kind kind = tolower((unsigned char)directive) == 'a'
                    ? FORMAT_SEG_AESTHETIC
                    : FORMAT_SEG_DECIMAL;

                if (litbuf_flush(&lit, out) != 0 || spec_push_seg(out, kind, NULL) != 0)
                    status = FORMAT_SPEC_NOMEM;
                break;
            }

            default:
                // Includes '\0': a control string ending in a stray '~'
                if (bad_offset != NULL)
                    *bad_offset = i;
                if (bad_directive != NULL)
                    *bad_directive = directive;

                status = FORMAT_SPEC_BAD;
                break;
        }

        i += 2;
    }

    if (status == FORMAT_SPEC_OK && litbuf_flush(&lit, out) != 0)
        status = FORMAT_SPEC_NOMEM;

    if (status != FORMAT_SPEC_OK) {
        free(lit.data);
        format_spec_free(out);
    }

    return status;
}

void
format_spec_free(struct format_spec *spec)
{
    if (spec == NULL)
        return;

    for (size_t i = 0; i < spec->count; ++i)
        free(spec->segs[i].text);

    free(spec->segs);

    spec->segs = NULL;
    spec->count = 0;
    spec->capacity = 0;
    spec->directive_count = 0;
}
