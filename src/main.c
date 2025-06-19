#include <stdio.h>
#include <stdlib.h>

void
usage(char *progname)
{
    printf("Error: Insufficient arguments provided | Usage: %s <file.lisp>\n", progname);
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc != 2)
        usage(argv[0]);

    FILE *srcp = fopen(argv[0], "r");
    if (srcp == NULL) {
        printf("Encountered I/O Error: Unable to read %s\n", argv[0]);
        return 1;
    }

    char *buffer;
    char *tokens[];

    return 0;
}
