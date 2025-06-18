#include <stdio.h>

int
lisp_entry()
{
    return 42;
}

int
main(int argc, char **argv)
{
    printf("%d\n", lisp_entry());
    return 0;
}