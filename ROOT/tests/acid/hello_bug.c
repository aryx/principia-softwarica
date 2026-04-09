#include <u.h>
#include <libc.h>

int foo(int x, int y) { 
    return x / y;
}

int bar(int a, int b) {
    return foo(a, b) + 3;
}

void
main(int argc, char **argv)
{
    // I use argc below otherwise 8c could optimize things.
    // I also use intermediate function calls with parameters instead
    // of computing locally z = x / y; because
    // 8c optimize such things and does not even allocate stack space
    // for locals that can fit in registers.
    USED(argv);

    int x, y, z;
    x = 41 + argc;
    y = argc - 1;
    z = bar(x, y);
    print("%d %d %d\n", x, y, z);
    exits(nil);
}
