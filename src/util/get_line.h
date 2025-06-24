/* Library functions written by Braden Best */
/* https://gitlab.com/bradenbest/bradenlib/ */

#ifndef MINI_LISP_X86_SRC_UTIL_GET_LINE_H_
#define MINI_LIST_X86_SRC_UTIL_GET_LINE_H_ 1000004L

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

enum get_line_flags {
    GET_LINE_FLAG_NONE             = 0x0,
    GET_LINE_FLAG_DISCARD_TRAILING = 0x1,
    GET_LINE_FLAG_END
};

static INLINE size_t get_line_from            (char *buffer, size_t bufsz, FILE *file);
static INLINE size_t get_line_from_greedy     (char *buffer, size_t bufsz, FILE *file);
static INLINE size_t get_line                 (char *buffer, size_t bufsz);
static INLINE size_t get_line_greedy          (char *buffer, size_t bufsz);
static INLINE size_t get_line_discard         (void);
static INLINE size_t get_line_implementation  (char *buffer, size_t bufsz, FILE *file, int flags);

static INLINE size_t
get_line_from(char *buffer, size_t bufsz, FILE *file)
{
    return get_line_implementation(buffer, bufsz, file, GET_LINE_FLAG_DISCARD_TRAILING);
}

static INLINE size_t
get_line_from_greedy(char *buffer, size_t bufsz, FILE *file)
{
    return get_line_implementation(buffer, bufsz, file, GET_LINE_FLAG_NONE);
}

static INLINE size_t
get_line(char *buffer, size_t bufsz)
{
    return get_line_from(buffer, bufsz, stdin);
}

static INLINE size_t
get_line_greedy(char *buffer, size_t bufsz)
{
    return get_line_from_greedy(buffer, bufsz, stdin);
}

static INLINE size_t
get_line_discard(void)
{
    return get_line(NULL, 0);
}

/*
 * 1. the loop conditions are a little complicated, so I broke it up into
 *    two parts. The first part will fill the buffer while there is space
 *    unless zero characters are read or the character read is a newline
 *
 * 2. the second part checks that there is no space in the buffer AND the
 *    DISCARD_TRAILING flag is set. If both of these conditions are true,
 *    then the rest of the line is consumed.
 *
 * If refactoring this function, be sure that the behavior complies with
 * the description at the top of the file. Specifically...
 *
 *  - greedy mode (DISCARD_TRAILING flag not set) MUST read everything
 *    (except newline)
 *  - normal mode MUST discard the trailoff
 *  - discard mode MUST discard everything.
 *
 *  An example of a refactoring introducing a bug would be if the fread
 *  is moved into the while header. Doing this would always read a
 *  character, which would result in a character getting lost in greedy
 *  mode. (size 10 input "Hello World!\n" -> {"Hello Worl", 10}, {"!", 1})
 */
static INLINE size_t
get_line_implementation(char *buffer, size_t bufsz, FILE *file, int flags)
{
    static char ch;
    size_t nread;
    size_t buflen = 0;

    if (file == NULL || feof(file))
        goto exit_eof;

    if (ferror(file))
        goto exit_error;

    while (buflen < bufsz) { /* 1 */
        if ((nread = fread(&ch, sizeof ch, 1, file)) == 0 || ch == '\n')
            break;

        buffer[buflen++] = ch;
    }

    if (buflen >= bufsz && (flags & GET_LINE_FLAG_DISCARD_TRAILING)) /* 2 */
        while ((nread = fread(&ch, sizeof ch, 1, file)) > 0 && ch != '\n')
            ;

    return buflen;

exit_error:
    {
        printf("get_line_implementation: Encountered I/O error.\n");
        return 0;
    }

exit_eof:
    return 0;
}

#endif

