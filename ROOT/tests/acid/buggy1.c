/* buggy1.c */
#include <u.h>
#include <libc.h>

void
main(int argc, char **argv)
{
    int x = 42;
    int y = 0;
    int z = x / y;
    print("%d\n", z);
    exits(nil);
}
