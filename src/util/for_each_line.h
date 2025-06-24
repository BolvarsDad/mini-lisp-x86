/*
 * Name:
 *     for_each_line v1.0.2
 *
 * Synopsis:
 *     size_t  for_each_line_getline         (char *buffer, size_t bufsz, FILE *file, int flags);
 *     int     for_each_line_implementation  (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn, int flags);
 *     int     for_each_line                 (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn);
 *     int     for_each_line_greedy          (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn);
 *
 * Arguments:
 *     buffer: buffer where input will go
 *     bufsz:  max size of buffer
 *     file:   file to read from
 *     flags:  bitmask (see Flags)
 *     fn:     function to call on each line (see Callback)
 *
 * Flags:
 *     FOR_EACH_LINE_FLAG_DISCARD_TRAILING - if set, causes rest of line
 *     that could not fit into buffer to be discarded. the greedy variant
 *     does not set this flag. See also get_line.h
 *
 * Callback:
 *     void callback(char const *line, size_t len)
 *     is called on each line with the contents of the line. The line is
 *     not null-terminated, and instead the length is provided
 *
 * Description:
 *     `for_each_line_getline` is a copy of get_line_implementation from
 *     get_line v1.0.2. It reads a line of input into a buffer and
 *     returns the length of said buffer, discarding the excess if the
 *     discard flag is set.
 *
 *     `for_each_line_implementation` calls `for_each_line_getline` until
 *     an empty line is read and calls the callback function with the
 *     line and its length
 *
 *     `for_each_line` calls `for_each_line_implementation` with the
 *     discard flag set.
 *
 *     `for_each_line_greedy` calls `for_each_line_implementation` with
 *     the discard flag not set.
 *
 * Return Value:
 *     `for_each_line_getline` returns the number of bytes written to the
 *     buffer
 *
 *     `for_each_line_implementation` and all functions based on it
 *     return 1 if there is "more file" to read or 0 if there was an EOF,
 *     error or file is null.
 */

// DISCLAIMER: Library functions not written by me. They were written by a friend of mine.
// https://gitlab.com/bradenbest/bradenlib/

#ifndef MINI_LISP_X86_UTIL_FOR_EACH_LINE_H_
#define MINI_LISP_X86_UTIL_FOR_EACH_LINE_H_ 1000002L

#include <stdio.h>

#ifndef INLINE
#    define INLINE
#endif

#ifdef __STDC_VERSION__
#    if __STDC_VERSION__ >= 199901L
#        undef INLINE
#        define INLINE inline
#    endif
#endif

typedef void (*for_each_line_fn)(char const *line, size_t len);

enum for_each_line_flags {
    FOR_EACH_LINE_FLAG_NONE             = 0x0,
    FOR_EACH_LINE_FLAG_DISCARD_TRAILING = 0x1,
    FOR_EACH_LINE_FLAG_END
};

static INLINE size_t  for_each_line_getline         (char *buffer, size_t bufsz, FILE *file, int flags);
static INLINE int     for_each_line_implementation  (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn, int flags);
static INLINE int     for_each_line                 (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn);
static INLINE int     for_each_line_greedy          (char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn);

/* Copied from get_line v1.0.2
 */
static INLINE size_t
for_each_line_getline(char *buffer, size_t bufsz, FILE *file, int flags)
{
    static char ch;
    size_t nread;
    size_t buflen = 0;

    if (file == NULL || feof(file))
        goto exit_eof;

    if (ferror(file))
        goto exit_error;

    while (buflen < bufsz) {
        if ((nread = fread(&ch, sizeof ch, 1, file)) == 0 || ch == '\n')
            break;

        buffer[buflen++] = ch;
    }

    if (buflen >= bufsz && (flags & FOR_EACH_LINE_FLAG_DISCARD_TRAILING))
        while ((nread = fread(&ch, sizeof ch, 1, file)) > 0 && ch != '\n')
            ;

    return buflen;

exit_error:
    {
        printf("for_each_line: Encountered I/O error.\n");
        return 0;
    }

exit_eof:
    return 0;
}

static INLINE int
for_each_line_implementation(char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn, int flags)
{
    size_t buflen;

    while ((buflen = for_each_line_getline(buffer, bufsz, file, flags)) > 0)
        fn(buffer, buflen);

    if (file == NULL || feof(file) || ferror(file))
        return 0;

    return 1;
}

static INLINE int
for_each_line(char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn)
{
    return for_each_line_implementation(buffer, bufsz, file, fn, FOR_EACH_LINE_FLAG_DISCARD_TRAILING);
}

static INLINE int
for_each_line_greedy(char *buffer, size_t bufsz, FILE *file, for_each_line_fn fn)
{
    return for_each_line_implementation(buffer, bufsz, file, fn, FOR_EACH_LINE_FLAG_NONE);
}

#endif

