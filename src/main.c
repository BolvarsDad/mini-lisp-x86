#include <stdio.h>
#include <stdlib.h>

#include "./compiler/lexer.h"
#include "./util/for_each_line.h"
#include "./util/get_line.h"

#define BUFSZ 4096

void
usage(char *progname)
{
    printf("Error: Insufficient arguments provided | Usage: %s <file.lisp>\n", progname);
    exit(1);
}

void
print_line(char const *line, size_t len)
{
    printf("%.*s\n", (int)len, line);
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);

    FILE *fp= fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Encountered I/O Error: Unable to read %s\n", argv[1]);
        return 1;
    }

    char buffer[BUFSZ];

    while (for_each_line(buffer, BUFSZ, fp, tokenize))
        ;

    return 0;
}
