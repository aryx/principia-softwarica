/* buggy.c */
#include <u.h>
#include <libc.h>

//void foo(int x) { }

void
main(int argc, char **argv)
{
    // use argc otherwise 8c will optimize things and 
    // not even allocate stack space for the locals

    int x, y, z;
    x = 41 + argc;
    y = argc - 1;
    z = x / y;
    print("%d %d %d\n", x, y, z);
    exits(nil);
}
